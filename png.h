/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
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

#ifndef __PNG_H
#define __PNG_H

#include "fz.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PNG_CN_IHDR 0x49484452
#define PNG_CN_PLTE 0x504C5445
#define PNG_CN_IDAT 0x49444154
#define PNG_CN_IEND 0x49454E44

const char* png_error_get(void);
int png_error_unsupported_get(void);
void png_error(const char* s, ...);
void png_error_unsupported(const char* s, ...);

int png_read_chunk(FZ* f, unsigned char** data, unsigned* size, unsigned* type);
int png_write_chunk(FZ* f, unsigned type, const unsigned char* data, unsigned size, unsigned* count);

int png_read_signature(FZ* f);
int png_write_signature(FZ* f, unsigned* count);

int png_read_iend(FZ* f, const unsigned char* data, unsigned data_size, unsigned type);
int png_read_ihdr(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	FZ* f, const unsigned char* data, unsigned data_size
);
int png_read(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	FZ* f
);

void png_expand_4(unsigned width, unsigned height, unsigned char* ptr);
void png_expand_2(unsigned width, unsigned height, unsigned char* ptr);
void png_expand_1(unsigned width, unsigned height, unsigned char* ptr);
void png_unfilter_8(unsigned width, unsigned height, unsigned char* ptr, unsigned line);
void png_unfilter_24(unsigned width, unsigned height, unsigned char* ptr, unsigned line);
void png_unfilter_32(unsigned width, unsigned height, unsigned char* ptr, unsigned line);

#ifdef __cplusplus
};
#endif

#endif
