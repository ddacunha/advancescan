/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "portable.h"

#include "zip.h"
#include "zlib.h"
#include "siglock.h"
#include "utility.h"
#include "endianrw.h"

#include <iostream>
#include <string>
#include <sstream>
#include <set>

#include <cerrno>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// -------------------------------------------------------------------------

bool ecd_compare_sig(const unsigned char *buffer) {
	static char ecdsig[] = { 'P', 'K', 0x05, 0x06 };
	return memcmp(buffer, ecdsig, 4) == 0;
}

// Locate end-of-central-dir sig in buffer and return offset
// out:
//   offset offset of cent dir start in buffer
// return:
//   true found;
bool ecd_find_sig (const unsigned char *buffer, unsigned buflen, unsigned& offset) {
	for (int i=buflen-22; i>=0; i--) {
		if (ecd_compare_sig(buffer+i)) {
			offset = i;
			return true;
		}
	}
	return false;
}

// Read cent dir and end cent dir data
// in:
//   f file
//   length length of file

#define ECD_READ_BUFFER_SIZE 4096

bool cent_read(FILE* f, unsigned length, unsigned char*& data, unsigned& size) {
	unsigned buf_length;

	if (length <= ECD_READ_BUFFER_SIZE) {
		buf_length = length;
	} else {
		// align the read
		buf_length = length - ((length - ECD_READ_BUFFER_SIZE) & ~(ECD_READ_BUFFER_SIZE-1));
	}

	while (true) {
		if (buf_length > length)
			buf_length = length;

		if (fseek(f, length - buf_length, SEEK_SET) != 0) {
			return false;
		}

		// allocate buffer
		unsigned char* buf = data_alloc( buf_length );
		assert( buf );

		if (fread( buf, buf_length, 1, f ) != 1) {
			data_free(buf);
			return false;
		}

		unsigned offset = 0;
		if (ecd_find_sig(buf, buf_length, offset)) {
			unsigned start_of_cent_dir = le_uint32_read( buf + offset + ZIP_EO_offset_to_start_of_cent_dir );
			unsigned buf_pos = length - buf_length;

			if (start_of_cent_dir >= length) {
				data_free(buf);
				return false;
			}

			size = length - start_of_cent_dir;

			data = data_alloc( size );
			assert( data );

			if (buf_pos <= start_of_cent_dir) {
				memcpy(data, buf + (start_of_cent_dir - buf_pos), size);
				data_free(buf);
			} else {
				data_free(buf);

				if (fseek(f, start_of_cent_dir, SEEK_SET) != 0) {
					data_free(data);
					data = 0;
					return false;
				}

				if (fread( data, size, 1, f ) != 1) {
					data_free(data);
					data = 0;
					return false;
				}
			}
			return true;
		}

		data_free(buf);

		if (buf_length < 8 * ECD_READ_BUFFER_SIZE && buf_length < length) {
			/* grow buffer */
			buf_length += ECD_READ_BUFFER_SIZE;
		} else {
			/* aborting */
			return false;
		}
	}
}

// Code used for disk entry
#define ZIP_UNIQUE_DISK 0

// Convert time_t to zip format
void time2zip(time_t tod, unsigned& date, unsigned& time) {
	struct tm* tm = gmtime(&tod);
	assert( tm );
	unsigned day = tm->tm_mday; // 1-31
	unsigned mon = tm->tm_mon + 1; // 1-12
	unsigned year = tm->tm_year - 80; // since 1980
	date = (day & 0x1F) | ((mon & 0xF) << 5) | ((year & 0x7F) << 9);
	unsigned sec = tm->tm_sec / 2; // 0-29, double to get real seconds
	unsigned min = tm->tm_min; // 0-59
	unsigned hour = tm->tm_hour; // 0-23
	time = (sec & 0x1F) | ((min & 0x3F) << 5) | ((hour & 0x1F) << 11);
}

// Convert zip time to to time_t
time_t zip2time(unsigned date, unsigned time) {
	struct tm tm;
	// reset all entry
	memset(&tm,0,sizeof(tm));
	// set know entry
	tm.tm_mday = date & 0x1F; // 1-31
	tm.tm_mon = ((date >> 5) & 0xF) - 1; // 0-11
	tm.tm_year = ((date >> 9) & 0x7f) + 80; // since 1900
	tm.tm_sec = (time & 0x1F) * 2; // 0-59
	tm.tm_min = (time >> 5) & 0x3F; // 0-59
	tm.tm_hour = (time >> 11) & 0x1F; // 0-23
	return mktime(&tm);
}

// -------------------------------------------------------------------------
// zip_entry

zip_entry::zip_entry(const zip& Aparent) {
	memset(&info,0xFF,sizeof(info));

	parent_name = Aparent.file_get();

	info.filename_length = 0;
	file_name = 0;

	info.local_extra_field_length = 0;
	local_extra_field = 0;

	info.central_extra_field_length = 0;
	central_extra_field = 0;

	info.file_comment_length = 0;
	file_comment = 0;

	info.compressed_size = 0;
	data = 0;
}

zip_entry::zip_entry(const zip_entry& A) {
	info = A.info;
	parent_name = A.parent_name;
	file_name = data_dup(A.file_name, info.filename_length);;
	local_extra_field = data_dup(A.local_extra_field, info.local_extra_field_length);
	central_extra_field = data_dup(A.central_extra_field, info.central_extra_field_length);
	file_comment = data_dup(A.file_comment, info.file_comment_length);
	data = data_dup( A.data, A.info.compressed_size );
}

zip_entry::~zip_entry() {
	data_free(file_name);
	data_free(local_extra_field);
	data_free(central_extra_field);
	data_free(file_comment);
	data_free(data);
}

zip_entry::method_t zip_entry::method_get() const {
	switch (info.compression_method) {
		case ZIP_METHOD_STORE :
			return store;
		case ZIP_METHOD_SHRUNK :
			return shrunk;
		case ZIP_METHOD_REDUCE1 :
			return reduce1;
		case ZIP_METHOD_REDUCE2 :
			return reduce2;
		case ZIP_METHOD_REDUCE3 :
			return reduce3;
		case ZIP_METHOD_REDUCE4 :
			return reduce4;
		case ZIP_METHOD_IMPLODE :
			switch (info.general_purpose_bit_flag & ZIP_GEN_FLAGS_IMPLODE_MASK) {
				case ZIP_GEN_FLAGS_IMPLODE_4KD2T :
					return implode_4kdict_2tree;
				case ZIP_GEN_FLAGS_IMPLODE_8KD2T :
					return implode_8kdict_2tree;
				case ZIP_GEN_FLAGS_IMPLODE_4KD3T :
					return implode_4kdict_3tree;
				case ZIP_GEN_FLAGS_IMPLODE_8KD3T :
					return implode_8kdict_3tree;
			}
			return unknow;
		case ZIP_METHOD_DEFLATE :
			switch (info.general_purpose_bit_flag & ZIP_GEN_FLAGS_DEFLATE_MASK) {
				case ZIP_GEN_FLAGS_DEFLATE_NORMAL :
					return deflate6;
				case ZIP_GEN_FLAGS_DEFLATE_MAXIMUM :
					return deflate9;
				case ZIP_GEN_FLAGS_DEFLATE_FAST :
					return deflate3;
				case ZIP_GEN_FLAGS_DEFLATE_SUPERFAST :
					return deflate1;
			}
			return unknow;
		case ZIP_METHOD_BZIP2 :
			return bzip2;
		case ZIP_METHOD_LZMA :
			return lzma;
	}

	return unknow;
}

bool zip_entry::is_text() const {
	return (info.internal_file_attrib & ZIP_INT_ATTR_TEXT) != 0;
}

void zip_entry::compressed_seek(FILE* f) const {

	// seek to local header
	if (fseek(f, offset_get(), SEEK_SET)!=0) {
		throw error() << "Failed seek " << parentname_get();
	}

	// read local header
	unsigned char buf[ZIP_LO_FIXED];
	if (fread(buf, ZIP_LO_FIXED, 1, f)!=1) {
		throw error() << "Failed read " << parentname_get();
	}

	check_local(buf);

	// may differ in strange zipfile
	unsigned local_extra_field_length = le_uint16_read(buf+ZIP_LO_extra_field_length);

	// seek to data
	if (fseek(f, info.filename_length + local_extra_field_length, SEEK_CUR) != 0) {
		throw error() << "Failed seek " << parentname_get();
	}
}

void zip_entry::compressed_read(unsigned char* outdata) const {

	if (data) {
		memcpy(outdata,data, compressed_size_get());
	} else {
		FILE* f = fopen(parentname_get().c_str(), "rb");
		if (!f) {
			throw error() << "Failed open for reading " << parentname_get();
		}

		try {
			compressed_seek(f);

			if (compressed_size_get() > 0) {
				if (fread(outdata,compressed_size_get(),1,f)!=1) {
					throw error() << "Failed read " << parentname_get();
				}
			}

		} catch (...) {
			fclose(f);
			throw;
		}

		fclose(f);
	}
}

time_t zip_entry::time_get() const {
	return zip2time(info.last_mod_file_date,info.last_mod_file_time);
}

void zip_entry::time_set(time_t tod) {
	time2zip(tod,info.last_mod_file_date,info.last_mod_file_time);
}

void zip_entry::set(method_t method, const string& Aname, const unsigned char* compdata, unsigned compsize, unsigned size, unsigned crc, unsigned date, unsigned time, bool is_text) {
	info.version_needed_to_extract = 20; // Version 2.0
	info.os_needed_to_extract = 0;
	info.version_made_by = 20; // Version 2.0
	info.host_os = 0;
	info.general_purpose_bit_flag = 0; // default
	info.last_mod_file_date = date;
	info.last_mod_file_time = time;
	info.crc32 = crc;
	info.compressed_size = compsize;
	info.uncompressed_size = size;
	info.internal_file_attrib = is_text ? ZIP_INT_ATTR_TEXT : 0;
	info.external_file_attrib = 0;

	switch (method) {
		case store :
			if (size != compsize) {
				throw error() << "Zip entry size mismatch";
			}
			info.compression_method = ZIP_METHOD_STORE;
			info.version_needed_to_extract = 10; // Version 1.0
		break;
		case shrunk :
			info.compression_method = ZIP_METHOD_SHRUNK;
		break;
		case reduce1 :
			info.compression_method = ZIP_METHOD_REDUCE1;
		break;
		case reduce2 :
			info.compression_method = ZIP_METHOD_REDUCE2;
		break;
		case reduce3 :
			info.compression_method = ZIP_METHOD_REDUCE3;
		break;
		case reduce4 :
			info.compression_method = ZIP_METHOD_REDUCE4;
		break;
		case deflate1 :
		case deflate2 :
			info.compression_method = ZIP_METHOD_DEFLATE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_DEFLATE_SUPERFAST;
		break;
		case deflate3 :
		case deflate4 :
		case deflate5 :
			info.compression_method = ZIP_METHOD_DEFLATE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_DEFLATE_FAST;
		break;
		case deflate6 :
		case deflate7 :
		case deflate8 :
			info.compression_method = ZIP_METHOD_DEFLATE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_DEFLATE_NORMAL;
		break;
		case deflate9 :
			info.compression_method = ZIP_METHOD_DEFLATE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
		break;
		case implode_4kdict_2tree :
			info.compression_method = ZIP_METHOD_IMPLODE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_IMPLODE_4KD2T;
		break;
		case implode_8kdict_2tree :
			info.compression_method = ZIP_METHOD_IMPLODE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_IMPLODE_8KD2T;
		break;
		case implode_4kdict_3tree :
			info.compression_method = ZIP_METHOD_IMPLODE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_IMPLODE_4KD3T;
		break;
		case implode_8kdict_3tree :
			info.compression_method = ZIP_METHOD_IMPLODE;
			info.general_purpose_bit_flag |= ZIP_GEN_FLAGS_IMPLODE_8KD3T;
		break;
		case bzip2 :
			info.compression_method = ZIP_METHOD_BZIP2;
		break;
		case lzma :
			info.compression_method = ZIP_METHOD_LZMA;
		break;
		default :
			throw error() << "Compression method not supported";
	}

	data_free(data);
	info.compressed_size = compsize;
	data = data_dup(compdata, info.compressed_size);

	name_set(Aname);

	data_free(local_extra_field);
	info.local_extra_field_length = 0;
	local_extra_field = 0;

	data_free(central_extra_field);
	info.central_extra_field_length = 0;
	central_extra_field = 0;

	data_free(file_comment);
	info.file_comment_length = 0;
	file_comment = 0;
}

void zip_entry::name_set(const string& Aname) {
	data_free(file_name);
	info.filename_length = Aname.length();
	file_name = data_alloc(info.filename_length);
	memcpy(file_name, Aname.c_str(), info.filename_length);
}

string zip_entry::name_get() const {
	return string(file_name, file_name + info.filename_length);
}

// Check central directory entry
void zip_entry::check_cent(const unsigned char* buf) const {
	// check signature
	if (le_uint32_read(buf+ZIP_CO_central_file_header_signature) != ZIP_C_signature) {
		throw error() << "Invalid central directory signature";
	}

	// check filename_length > 0, can't exist a file without a name
	if (le_uint16_read(buf+ZIP_CO_filename_length) == 0) {
		throw error() << "Empty filename in central directory";
	}
}

// Check local file header comparing with internal information
void zip_entry::check_local(const unsigned char* buf) const {
	if (le_uint32_read(buf+ZIP_LO_local_file_header_signature) != ZIP_L_signature) {
		throw error() << "Invalid signature in local header";
	}
#if 0 // may differ in strange zips, it can be safely ignored
	if (info.general_purpose_bit_flag != le_uint16_read(buf+ZIP_LO_general_purpose_bit_flag)) {
		throw error() << "Invalid local purpose bit flag";
	}
#endif
	if (info.compression_method != le_uint16_read(buf+ZIP_LO_compression_method)) {
		throw error() << "Invalid method on local header";
	}
	if (info.crc32 != le_uint32_read(buf+ZIP_LO_crc32)) {
		throw error() << "Invalid crc on local header " << info.crc32 << " " << le_uint32_read(buf+ZIP_LO_crc32);
	}
	if (info.compressed_size != le_uint32_read(buf+ZIP_LO_compressed_size)) {
		throw error() << "Invalid compressed size in local header";
	}
	if (info.uncompressed_size != le_uint32_read(buf+ZIP_LO_uncompressed_size)) {
		throw error() << "Invalid  uncompressed size in local header";
	}
	if (info.filename_length != le_uint16_read(buf+ZIP_LO_filename_length)) {
		throw error() << "Invalid filename in local header";
	}
#if 0 // the .zip generated with the info-zip program have the extra field only on the local header
	if (info.extra_field_length != le_uint16_read(buf+ZIP_LO_extra_field_length)) {
		throw error() << "Invalid extra field length in local header";
	}
#endif
}

// Unload compressed/uncomressed data
void zip_entry::unload() {
	data_free(data);
	data = 0;
}

// Skip local file header and data
void zip::skip_local(const unsigned char* buf, FILE* f) {
	unsigned local_extra_field_length = le_uint16_read(buf+ZIP_LO_extra_field_length);
	unsigned filename_length = le_uint16_read(buf+ZIP_LO_filename_length);
	unsigned compressed_size = le_uint32_read(buf+ZIP_LO_compressed_size);
	
	// skip filename and extra field
	if (fseek(f,filename_length + local_extra_field_length,SEEK_CUR)!=0) {
		throw error() << "Failed seek";
	}

	// directory don't have data
	if (compressed_size) {
		if (fseek(f,compressed_size,SEEK_CUR)!=0) {
			throw error() << "Failed seek";
		}
	}
}

// Load local file header
// in:
//   buf fixed size local header, sig not valid
//   f file seeked after the fixed size local header
// return:
//   true ok
void zip_entry::load_local(const unsigned char* buf, FILE* f) {
	check_local(buf);

	// use the local extra_field_length. It may be different than the
	// central directory version in some zips.
	unsigned local_extra_field_length = le_uint16_read(buf+ZIP_LO_extra_field_length);

	// skip filename and extra field
	if (fseek(f,info.filename_length + local_extra_field_length,SEEK_CUR)!=0) {
		throw error() << "Failed seek";
	}

	data_free(data);
	data = data_alloc(info.compressed_size);

	try {
		if (info.compressed_size > 0) {
			if (fread(data,info.compressed_size,1,f)!=1) {
				throw error() << "Failed read";
			}
		}
	} catch (...) {
		data_free(data);
		data = 0;
		throw;
	}
}

// Save local file header
// in:
//   f file seeked at correct position
// return:
//   true ok
void zip_entry::save_local(FILE* f) {
	long offset = ftell(f);

	if (offset<0)
		throw error() << "Failed tell";

	info.relative_offset_of_local_header = offset;

	// write header
	unsigned char buf[ZIP_LO_FIXED];
	le_uint32_write(buf+ZIP_LO_local_file_header_signature, ZIP_L_signature);
	le_uint8_write(buf+ZIP_LO_version_needed_to_extract,info.version_needed_to_extract);
	le_uint8_write(buf+ZIP_LO_os_needed_to_extract,info.os_needed_to_extract);
	le_uint16_write(buf+ZIP_LO_general_purpose_bit_flag,info.general_purpose_bit_flag);
	le_uint16_write(buf+ZIP_LO_compression_method,info.compression_method);
	le_uint16_write(buf+ZIP_LO_last_mod_file_time,info.last_mod_file_time);
	le_uint16_write(buf+ZIP_LO_last_mod_file_date,info.last_mod_file_date);
	le_uint32_write(buf+ZIP_LO_crc32,info.crc32);
	le_uint32_write(buf+ZIP_LO_compressed_size,info.compressed_size);
	le_uint32_write(buf+ZIP_LO_uncompressed_size,info.uncompressed_size);
	le_uint16_write(buf+ZIP_LO_filename_length,info.filename_length);
	le_uint16_write(buf+ZIP_LO_extra_field_length,info.local_extra_field_length);

	if (fwrite(buf,ZIP_LO_FIXED,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write filename
	if (fwrite(file_name,info.filename_length,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write the extra field
	if (info.local_extra_field_length && fwrite(local_extra_field,info.local_extra_field_length,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write data, directories don't have data
	if (info.compressed_size) {
		assert( data );

		if (fwrite(data,info.compressed_size,1,f)!=1) {
			throw error() << "Failed write";
		}
	}
}

// Load cent dir
// in:
//   buf fixed size cent dir
//   f file seeked after the fixed size cent dir
// return:
//   true ok
//   false the object MUST be destroyed
void zip_entry::load_cent(const unsigned char* _buf, unsigned& skip) {
	const unsigned char* buf = _buf;

	check_cent(buf);

	// read header
	info.version_made_by = le_uint8_read(buf+ZIP_CO_version_made_by);
	info.host_os = le_uint8_read(buf+ZIP_CO_host_os);
	info.version_needed_to_extract = le_uint8_read(buf+ZIP_CO_version_needed_to_extract);
	info.os_needed_to_extract = le_uint8_read(buf+ZIP_CO_os_needed_to_extract);
	info.general_purpose_bit_flag = le_uint16_read(buf+ZIP_CO_general_purpose_bit_flag);
	info.compression_method = le_uint16_read(buf+ZIP_CO_compression_method);
	info.last_mod_file_time = le_uint16_read(buf+ZIP_CO_last_mod_file_time);
	info.last_mod_file_date = le_uint16_read(buf+ZIP_CO_last_mod_file_date);
	info.crc32 = le_uint32_read(buf+ZIP_CO_crc32);
	info.compressed_size = le_uint32_read(buf+ZIP_CO_compressed_size);
	info.uncompressed_size = le_uint32_read(buf+ZIP_CO_uncompressed_size);
	info.filename_length = le_uint16_read(buf+ZIP_CO_filename_length);
	info.central_extra_field_length = le_uint16_read(buf+ZIP_CO_extra_field_length);
	info.file_comment_length = le_uint16_read(buf+ZIP_CO_file_comment_length);
	info.internal_file_attrib = le_uint16_read(buf+ZIP_CO_internal_file_attrib);
	info.external_file_attrib = le_uint32_read(buf+ZIP_CO_external_file_attrib);
	info.relative_offset_of_local_header = le_uint32_read(buf+ZIP_CO_relative_offset_of_local_header);
	buf += ZIP_CO_FIXED;

	// read filename
	data_free(file_name);
	file_name = data_alloc(info.filename_length);
	memcpy( file_name, buf, info.filename_length );
	buf += info.filename_length;

	// read extra field
	data_free(central_extra_field);
	central_extra_field = data_dup(buf, info.central_extra_field_length);
	buf += info.central_extra_field_length;

	// read comment
	data_free(file_comment);
	file_comment = data_dup( buf, info.file_comment_length);
	buf += info.file_comment_length;

	skip = buf - _buf;
}

// Save cent dir
// in:
//   f file seeked at correct position
// return:
//   true ok
void zip_entry::save_cent(FILE* f) {
	unsigned char buf[ZIP_CO_FIXED];

	le_uint32_write(buf+ZIP_CO_central_file_header_signature, ZIP_C_signature);
	le_uint8_write(buf+ZIP_CO_version_made_by,info.version_made_by);
	le_uint8_write(buf+ZIP_CO_host_os,info.host_os);
	le_uint8_write(buf+ZIP_CO_version_needed_to_extract,info.version_needed_to_extract);
	le_uint8_write(buf+ZIP_CO_os_needed_to_extract,info.os_needed_to_extract);
	le_uint16_write(buf+ZIP_CO_general_purpose_bit_flag,info.general_purpose_bit_flag);
	le_uint16_write(buf+ZIP_CO_compression_method,info.compression_method);
	le_uint16_write(buf+ZIP_CO_last_mod_file_time,info.last_mod_file_time);
	le_uint16_write(buf+ZIP_CO_last_mod_file_date,info.last_mod_file_date);
	le_uint32_write(buf+ZIP_CO_crc32,info.crc32);
	le_uint32_write(buf+ZIP_CO_compressed_size,info.compressed_size);
	le_uint32_write(buf+ZIP_CO_uncompressed_size,info.uncompressed_size);
	le_uint16_write(buf+ZIP_CO_filename_length,info.filename_length);
	le_uint16_write(buf+ZIP_CO_extra_field_length,info.central_extra_field_length);
	le_uint16_write(buf+ZIP_CO_file_comment_length,info.file_comment_length);
	le_uint16_write(buf+ZIP_CO_disk_number_start, ZIP_UNIQUE_DISK );
	le_uint16_write(buf+ZIP_CO_internal_file_attrib,info.internal_file_attrib);
	le_uint32_write(buf+ZIP_CO_external_file_attrib,info.external_file_attrib);
	le_uint32_write(buf+ZIP_CO_relative_offset_of_local_header,info.relative_offset_of_local_header);

	if (fwrite(buf,ZIP_CO_FIXED,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write filename
	if (fwrite(file_name,info.filename_length,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write extra field
	if (info.central_extra_field_length && fwrite(central_extra_field,info.central_extra_field_length,1,f)!=1) {
		throw error() << "Failed write";
	}

	// write comment
	if (info.file_comment_length && fwrite(file_comment,info.file_comment_length,1,f)!=1) {
		throw error() << "Failed write";
	}
}

// -------------------------------------------------------------------------
// Zip

bool zip::pedantic = false;

zip::zip(const std::string& Apath) : path(Apath)  {
	flag.open = false;
	flag.read = false;
	flag.modify = false;
	zipfile_comment = 0;
}

zip::zip(const zip& A) : map(A.map), path(A.path) {
	flag = A.flag;
	info = A.info;
	zipfile_comment = data_dup( A.zipfile_comment, A.info.zipfile_comment_length );
}

zip::~zip() {
	if (is_open())
		close();
}

void zip::create() {
	assert( !flag.open );

	info.offset_to_start_of_cent_dir = 0;
	info.zipfile_comment_length = 0;
	data_free(zipfile_comment);
	zipfile_comment = 0;

	flag.read = true;
	flag.open = true;
	flag.modify = false;
}

void zip::open() {
	assert( !flag.open );

	struct stat s;
	if (stat(path.c_str(), &s)!=0) {
		if (errno != ENOENT)
			throw error() << "Failed stat";

		// create the file if it's missing

		create();
		return;
	}

	unsigned length = s.st_size;

	// open file
	FILE* f = fopen(path.c_str(),"rb");
	if (!f)
		throw error() << "Failed open for reading";

	unsigned char* data = 0;
	unsigned data_size = 0;

	try {
		if (!cent_read(f, length, data, data_size))
			throw error() << "Failed read end of central directory";
	} catch (...) {
		fclose(f);
		throw;
	}

	fclose(f);

	// position in data
	unsigned data_pos = 0;

	try {
		// central dir
		while (le_uint32_read(data+data_pos) == ZIP_C_signature) {

			iterator i = map.insert(map.end(), zip_entry(path) );

			unsigned skip = 0;
			try {
				i->load_cent(data + data_pos, skip);
			} catch (...) {
				map.erase(i);
				throw;
			}

			data_pos += skip;
		}

		// end of central dir
		if (le_uint32_read(data+data_pos) != ZIP_E_signature)
			throw error() << "Invalid end of central dir signature";

		info.offset_to_start_of_cent_dir = le_uint32_read(data+data_pos+ZIP_EO_offset_to_start_of_cent_dir);
		info.zipfile_comment_length = le_uint16_read(data+data_pos+ZIP_EO_zipfile_comment_length);
		data_pos += ZIP_EO_FIXED;

		if (info.offset_to_start_of_cent_dir != length - data_size)
			throw error() << "Invalid end of central directory start address";

		// comment
		data_free(zipfile_comment);
		zipfile_comment = data_dup( data+data_pos, info.zipfile_comment_length);
		data_pos += info.zipfile_comment_length;

	} catch (...) {
		data_free(data);
		throw;
	}

	// delete cent data
	data_free(data);

	if (pedantic) {
		// don't accept garbage at the end of file
		if (data_pos != data_size)
			throw error() << data_size - data_pos << " unused bytes at the end of the central directory";
	}

	flag.open = true;
	flag.read = false;
	flag.modify = false;
}

// Close a zip file
void zip::close() {

	// deinizialize
	flag.open = false;
	flag.read = false;
	flag.modify = false;
	data_free(zipfile_comment);
	zipfile_comment = 0;
	path = "";
	map.erase( map.begin(), map.end() );
}

// Discarge compressed data read
// note:
//   you cannot call unload() if zip is modified, you must call reopen()
void zip::unload() {
	assert( flag.open && flag.read && !flag.modify );

	for(iterator i=begin();i!=end();++i)
		i->unload();

	flag.read = false;
}

// Reopen from zip on disk, lose any modify
// note:
//   equivalent close and open
void zip::reopen() {
	assert( flag.open );

	close();
	open();
}

// Load a zip file
// return:
//   true ok
void zip::load() {
	assert( flag.open && !flag.read );

	flag.modify = false;

	FILE* f = fopen(path.c_str(),"rb");
	if (!f)
		throw error() << "Failed open for reading";

	try {
		long offset = 0;
		unsigned count = 0;

		while (static_cast<unsigned long>(offset) < info.offset_to_start_of_cent_dir) {
			unsigned char buf[ZIP_LO_FIXED];

			if (fread(buf,ZIP_LO_FIXED,1,f)!=1)
				throw error() << "Failed read";

			if (ecd_compare_sig(buf))
				throw error() << "Invalid local header, maybe a central directory";

			// search item
			iterator next;
			bool next_set = false;
			for(iterator i=begin();i!=end();++i) {
				if (i->offset_get() >= static_cast<unsigned long>(offset)) {
					if (!next_set || i->offset_get() < next->offset_get()) {
						next_set = true;
						next = i;
					}
				}
			}

			if (!next_set) {
				if (pedantic)
					throw error() << info.offset_to_start_of_cent_dir - static_cast<unsigned long>(offset) << " unused bytes after the last local header at offset " << offset;
				else
					break;
			}

			// check for a data hole
			if (next->offset_get() > static_cast<unsigned long>(offset)) {
				if (pedantic)
					throw error() << next->offset_get() - static_cast<unsigned long>(offset) << " unused bytes at offset " << offset;
				else {
					// set the correct position
					if (fseek(f, next->offset_get(), SEEK_SET) != 0)
						throw error() << "Failed fseek";
					offset = next->offset_get();
				}
			}

			next->load_local(buf,f);

			++count;

			offset = ftell(f);
			if (offset < 0)
				throw error() << "Failed tell";
		}

		if (static_cast<unsigned long>(offset) != info.offset_to_start_of_cent_dir) {
			if (pedantic)
				throw error() << "Invalid central directory start";
		}
	
		if (count != size())
			throw error() << "Invalid central directory, expected " << size() << " local headers, got " << count;

	} catch (...) {
		fclose(f);
		throw;
	}

	fclose(f);

	flag.read = true;
}

unsigned zip::size_not_zero() const {
	unsigned count = 0;

	// check if it contains at least one file with size > 0 (directories are excluded)
	for(const_iterator i=begin();i!=end();++i) {
		if (i->uncompressed_size_get() > 0) {
			++count;
		}
	}

	return count;
}

// Save a zip file
// return:
//   true ok
void zip::save() {
	assert( flag.open && flag.read );

	flag.modify = false;

	if (size_not_zero() > 0) {
		// prevent external signal
		sig_auto_lock sal;

		// temp name of the saved file
		string save_path = file_basepath(path) + ".tmp";

		FILE* f = fopen(save_path.c_str(),"wb");
		if (!f)
			throw error() << "Failed open for writing of " << save_path;

		try {
			// write local header
			for(iterator i=begin();i!=end();++i)
				i->save_local(f);

			long cent_offset = ftell(f);
			if (cent_offset<0)
				throw error() << "Failed tell";

			// new cent start
			info.offset_to_start_of_cent_dir = cent_offset;

			// write cent dir
			for(iterator i=begin();i!=end();++i)
				i->save_cent(f);

			long end_cent_offset = ftell(f);
			if (end_cent_offset<0)
				throw error() << "Failed tell";

			// write end of cent dir
			unsigned char buf[ZIP_EO_FIXED];
			le_uint32_write(buf+ZIP_EO_end_of_central_dir_signature, ZIP_E_signature);
			le_uint16_write(buf+ZIP_EO_number_of_this_disk, ZIP_UNIQUE_DISK);
			le_uint16_write(buf+ZIP_EO_number_of_disk_start_cent_dir, ZIP_UNIQUE_DISK);
			le_uint16_write(buf+ZIP_EO_total_entries_cent_dir_this_disk, size());
			le_uint16_write(buf+ZIP_EO_total_entries_cent_dir, size());
			le_uint32_write(buf+ZIP_EO_size_of_cent_dir, end_cent_offset - cent_offset);
			le_uint32_write(buf+ZIP_EO_offset_to_start_of_cent_dir, cent_offset);
			le_uint16_write(buf+ZIP_EO_zipfile_comment_length, info.zipfile_comment_length);

			if (fwrite(buf,ZIP_EO_FIXED,1,f)!=1)
				throw error() << "Failed write";

			// write comment
			if (info.zipfile_comment_length && fwrite(zipfile_comment,info.zipfile_comment_length,1,f)!=1)
				throw error() << "Failed write";

		} catch (...) {
			fclose(f);
			remove(save_path.c_str());
			throw;
		}

		fclose(f);

		// delete the file if exists
		if (access(path.c_str(), F_OK) == 0) {
			if (remove(path.c_str()) != 0) {
				remove(save_path.c_str());
				throw error() << "Failed delete of " << path;
			}
		}

		// rename the new version with the correct name
		if (::rename(save_path.c_str(), path.c_str()) != 0) {
			throw error() << "Failed rename of " << save_path << " to " << path;
		}

	} else {
		// reset the cent start
		info.offset_to_start_of_cent_dir = 0;

		// delete the file if exists
		if (access(path.c_str(), F_OK) == 0) {
			if (remove(path.c_str()) != 0)
				throw error() << "Failed delete of " << path;
		}
	}
}

// Remove a file
void zip::erase(iterator i) {
	assert( flag.read );

	flag.modify = true;

	map.erase( i );
}

// Rename a file
// return:
//   true ok
// note:
//   no filename overwrite check
void zip::rename(iterator i, const string& Aname) {
	assert( flag.read );

	flag.modify = true;

	i->name_set(Aname);
}

/// Insert a zip entry.
zip::iterator zip::insert(const zip_entry& A, const string& Aname) {
	iterator i;

	assert( flag.read );

	unsigned char* data = data_alloc( A.compressed_size_get() );
	assert( data );

	try {
		A.compressed_read(data);

		i = map.insert( map.end(), zip_entry(path) );

		try {
			i->set( A.method_get(), Aname, data, A.compressed_size_get(), A.uncompressed_size_get(), A.crc_get(), A.zipdate_get(), A.ziptime_get(), A.is_text());
		} catch (...) {
			map.erase( i );
			throw;
		}

		flag.modify = true;

	} catch (...) {
		data_free(data);
		throw;
	}

	data_free(data);

	return i;
}

/// Add data to a zip file
// return:
//   !=end() ok
// note:
//   data is deflated or stored
//   no filename overwrite check
zip::iterator zip::insert_uncompressed(const string& Aname, const unsigned char* data, unsigned size, unsigned crc, time_t tod, bool is_text) {
	iterator i;

	assert( flag.read );
	assert( crc == crc32(0, (const unsigned char*)data, size) );

	unsigned date = 0;
	unsigned time = 0;
	time2zip(tod,date,time);

	i = map.insert( map.end(), zip_entry(path) );

	try {
		i->set(zip_entry::store, Aname, data, size, size, crc, date, time, is_text);
	} catch (...) {
		map.erase( i );
		throw;
	}

	flag.modify = true;

	return i;
}


