Name
	advscan - The AdvanceSCAN roms manager

Synopsis
	:advscan [-c, --cfg config.rc] [-r, --rom] [-s, --sample]
	:[-a, --add-zip] [-b, --add-bin] [-d, --del-zip]
	:[-u, --del-unknown] [-g, --del-garbage] [-t,--del-text]
	:[-n, --print-only] [-p, --report]
	:[-v, --verbose] < info.lst

	:advscan [-R, --rom-std] [-S, --sample-std] < info.lst

	:advscan [-i, --ident] files... < info.lst

	:advscan [-l, --bbs] < info.lst

	:advscan [-e, --equal] < info.lst

	:advscan [-h, --help] [-V, --version]

Description
	advscan is a command line utility for maintaining a rom
	and sample zip archive for the MAME, XMAME, AdvanceMAME
	and Raine emulators. The goal of advscan is to obtain a
	complete and perfect rom and sample zip archive with
	differential merging. Differential merging means that any
	game has his zip archive, which contains all the rom
	files, which are not already in the parent zip archive (if
	exists).

	advscan has these features:

	* Directly read, write zip archives without decompressing
		and recompressing them for the best performance.
	* Add, copy, move and rename files in the zip
		archives. Any rom that you have is placed
		automatically in the correct zip.
	* Recognize the text files added by rom sites and
		delete them.
	* Recognize the text files added by the rom dumpers
		and keep or delete them.
	* It's safe. On all the zip operations any file
		removed or overwritten is saved in the
		`rom_unknown' `sample_unknown' directories and kept
		for future uses. This will prevent any unwanted
		remove operation.

	but also has these misfeatures:

	* Support only rom and sample archives zipped.
	* Support only differential merging.

Options
	-c, --cfg config.rc
		Select the configuration file. If not specified is
		assumed the file `advscan.rc' in the current
		directory.

	=< info.lst
		To operate advscan needs always as input a rom
		information file. It can be generally created with
		the -listinfo (MAME) or -gameinfo (Raine) options
		of the emulator.

	-r,--rom
		Operates on roms. All the next commands will
		operate on your romset.

	-s,--sample
		Operates on samples. All the next commands will
		operate on your sampleset.

	-a,--add-zip
		Add the missing rom zips. Any missing zip archive
		for which at least one rom is available is created.

	-b,--add-bin
		Add, rename and substitute all the necessary files
		in the existing zip archives. No new zip archives
		are created, only the already present zips are
		modified. No file is deleted. Anyway, some files may
		be overwritten. Any file overwritten is saved in
		the `rom_unknown' or `sample_unknown' directory in
		a zip archive with the same name of the original
		one.

	-d,--del-zip
		Remove any unknown zip archive. Any archive
		removed is saved in the `rom_unknown' or
		`sample_unknown' directory.

	-u,--del-unknown
		Remove any unknown file from the existing zip
		archives. Any file removed is saved in the
		`rom_unknown' or `sample_unknown' directory in a
		zip archive with the same name of the original one.
		Only binary files are removed, all the text files
		are kept.

	-g,--del-garbage
		Remove any garbage file from the zip archives. A
		garbage file is an advertising text file added
		generally by a rom site. The files removed are not
		saved.

	-t,--del-text
		Remove any text file from the zip archives. Any
		file removed is saved in the `rom_unknown' or
		`sample_unknown' directory in a zip archive with the
		same name of the original one.

	-R,--rom-std
		Shortcut for the options -rabdug. It does all the
		previous operations on roms except removing text
		files.

	-S,--sample-std
		Shortcut for the options -sabdug. It does all the
		previous operations on samples except removing text
		files.

	-n,--print-only
		Don't modify anything, it only shows operations.
		This option prevents any changes made by the
		previous commands. The operations are only printed and
		NOT executed.

	-p,--report
		Write an extensive text report with the list of
		good, bad and missing roms or/and samples. The
		content of any zip archive with missing files is
		printed. You must also specify the -r or/and -s
		options.

	-v,--verbose
		Print a more verbose report. The content of any zip
		archive is printed also if it contains any not rom
		files.

Information Options
	The following options are used only to print information.
	These options don't need or read the configuration file.

	-i, --ident files...
		Identify the files specified. Only the information
		present in the info file is used.

	-l, --bbs
		Print a standard `.bbs' files with the description
		of all the roms in the info file.

	-e, --equal
		Print a list of all the duplicate roms present in
		the info file. Only the information present in the
		info file is used.

	-h, --help
		Print a short help screen.

	-V, --version
		Print the program version.

Identification
	Rom files are identified by their crc and size. The roms
	are not really decompressed, but the crc value stored on
	the zip archives is used. If a rom has an incorrect crc or
	size, but it has a correct name, it's maintained if
	doesn't exists a valid alternative.

	Sample files are identified only by their name. This
	limits the possible operations. Essentially advscan can
	report only missing samples.

	Garbage files are identified by their size and crc.

	All the others files are identified with this algo:

	* If the name is like *.sam, *.wav it's considered an
		unknown binary file.
	* If the name is something like *.doc, *.txt, *.nfo,
		*.diz, readme.* it's considered a text file.
	* If the size is a power of 2 it's considered an
		unknown binary file.
	* It's considered a text file.

Configuration
	To run advscan you need two files. The rom information
	file and the configuration file.

	The rom information file is the file that contains the
	information of all the roms used by the emulator. It can
	be made with the command:

		:advmame -listinfo > info.lst

	This file is expected as input of advscan. So, you can use
	this command:

		:advscan [options] < info.lst

	Or combine the two commands together:

		:advmame -listinfo | advscan [options]

	The configuration file is a text files that describes your
	directories structure. You can use absolute path or
	relative path. Relative path are relative to the current
	directory when you run advscan.

	On Unix the PATH separator is `:'. On DOS the PATH
	separator is `;'. The following options are expressed with the
	Unix format.

	=rom PATH:PATH...
		List of paths where the roms are placed. These are
		the zip archives, which are modified fixed.

	=rom_new PATH
		Single path where the new created zip archives are
		placed. It's STRONGLY suggested to put this path
		ALSO in the `rom' specification. Otherwise at the
		next run the zip archives are recreated.

	=rom_import PATH:PATH...
		List of directory tree where other roms files are
		placed. These are used for importing rom file missing
		in rompath. These files are only read and never
		modified in any way. It's very useful to insert
		here any rom directories of any other arcade
		emulators. When a new game will be supported the rom
		archive will be made automatically.

	=rom_unknown PATH
		Single path where unknown rom zip archives will be
		moved. In this directory is inserted any rom file
		removed from the rom zip archives. However, any rom
		file is automatically deleted by advscan if it's
		duplicated in an archive listed on the `rom' or
		`rom_import' options.

	=sample PATH:PATH...
		List of path where the samples are placed. These
		are the zip archives, which are modified and fixed.

	=sample_unknown PATH
		Single path where unknown sample zip archives will
		be moved. In this directory is inserted any sample
		file removed from the sample zip archives.

	If the -c option is not specified the configuration file
	is read from ./advscan.rc.

	The files advscan.rc.linux and advscan.rc.dos are two
	examples of configuration files.

Report
	The report generated with the -p option contains some text
	tag explicated here:

	=rom_good
		A recognized good rom. The rom is recognized by his
		name, crc and size.

	=rom_bad
		A recognized bad rom with an incorrect size or crc.
		The rom is recognized by his name.

	=rom_miss
		A missing rom.

	=nodump_miss
		A missing "NO GOOD DUMP KNOWN" rom.

	=nodump_bad
		A recognized "NO GOOD DUMP KNOWN" rom. The rom is
		recognized by his name and size.

	=redump_bad
		A recognized "ROM NEEDS REDUMP" rom. The rom is
		recognized by his name, size and negate crc. The
		negate crc is the special convention used by MAME
		to sign known bad dump.

	=text
		An unknown text file.

	=binary
		An unknown binary file.

	=garbage
		A recognized garbage file. A garbage file is an
		advertising text file added generally by a rom
		site. The file is recognized by his name, size and
		crc.

	=sound_good
		A recognized good sound sample. The sample is
		recognized by his name.

	=sound_miss
		A missing sound sample.

Examples
	For the generic use you need to run advscan with the
	options:

		:advscan -R < info.lst

	This command will fix your rom collection (without removing
	the precious text files).

	To check in advance all the operations that will be done
	you can use the command:

		:advscan -R -n < info.lst

	which show only the operations.

	To only generate an extensive report of your rom set you
	can use the command:

		:advscan -r -p < info.lst > report.txt

	To increase the verbosity of the information printed you
	can add the -v switch.

Copyright
	This file is Copyright (C) 2002 Andrea Mazzoleni

See Also
	advdiff(1)