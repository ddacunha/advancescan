Name{number}
	advscan - AdvanceSCAN Roms Manager

Synopsis
	:advscan [-c, --cfg CONFIG] [-r, --rom] [-s, --sample]
	:	[-k, --disk] [-a, --add-zip] [-b, --add-bin]
	:	[-d, --del-zip] [-u, --del-unknown]
	:	[-g, --del-garbage] [-t,--del-text]
	:	[-n, --print-only] [-p, --report]
	:	[-f, --filter FILTER]
	:	[-v, --verbose] < info.xml

	:advscan [-R, --rom-std] [-S, --sample-std]
	:	[-K, --disk-std]< info.xml

	:advscan [-i, --ident] files... < info.xml

	:advscan [-l, --bbs] < info.xml

	:advscan [-e, --equal] < info.xml

	:advscan [-h, --help] [-V, --version]

Description
	advscan is a command line utility for maintaining a zipped
	archive of roms and samples for AdvanceMAME, AdvanceMESS
	and any other MAME derivative emulator.

	The goal of advscan is to obtain a complete and perfect roms
	and samples archive with differential merging.

	Differential merging means that any game has its zip archive,
	which contains all the rom files, which are not already in
	the parent zip archive (if exists).

	advscan has these features:

	* Directly read, writes zip archives without decompressing
		and recompressing them for best performance.
	* Add, copy, move and rename files in the zip
		archives. Any rom that you have is placed
		automatically in the correct zip.
	* Recognizes the text files added by rom sites and
		deletes them.
	* Recognizes the text files added by the rom dumpers
		and keeps or deletes them as your choice.
	* It's safe. In all the zip operations any file
		removed or overwritten is saved in the
		`rom_unknown' and `sample_unknown' directories and keep
		for future uses. This will prevent any unwanted
		remove operation.

	but also has these misfeatures:

	* Supports only roms and samples zipped.
	* Supports only roms organized with differential merging.
	* Doesn't support .chd files in subdirectories.

Getting Started
	To use AdvanceSCAN you need first to create and edit its textual
	configuration file named 'advscan.rc'. An example advscan.rc file
	is provided in the distribution archive, but you can also write
	it from scratch.

	Let's start your preferred text editor and create the configuration
	file advscan.rc with these options:

		:rom DIR_OF_YOUR_ROMS
		:rom_new DIR_OF_YOUR_ROMS
		:rom_unknown DIR_OF_REJECTED_ROMS
		:rom_import DIR_OF_ROMS_TO_IMPORT

	The most important configuration option is the directory where
	your roms are stored. This directory must be specified in the
	'rom' and 'rom_new' options.
	The 'rom_unknown' option is used to select the directory where
	store the unused roms. AdvanceSCAN never deletes roms for safety,
	so you need to specify this 'basket' directory.
	With the 'rom_import' option you can specify the directory where
	the roms to import resides. For example the update packs you want
	to integrate in your roms. If you don't have any, omit this option.

	To analyze the roms, AdvanceSCAN needs also the roms
	information file from the emulator. This file can be created
	with the command:

		:mame -listxml > info.xml

	Finally, you can now run AdvanceSCAN to fix your roms with
	the command:

		:advscan -R < info.xml

	You can also generate a report of the state of your roms
	running the command:

		:advscan -r -p < info.xml > report.txt

Options
	-c, --cfg CONFIG
		Select the configuration file. If not specified is
		assumed the file `advscan.rc' in the current
		directory.

	=< info.xml
		To operate advscan needs always as input a rom
		information file. It can be generally created with
		the -listxml options of the emulator.

	-r, --rom
		Operates on roms (.zip files). All the next commands
		will operate on your roms.

	-s, --sample
		Operates on samples. All the next commands will
		operate on your samples.

	-k, --disk
		Operates on disks (.chd files). All the next commands
		will operate on your disks.

	-a, --add-zip
		Add the missing rom zips. Any missing zip archive
		for which at least one rom is available will be created.

	-b, --add-bin
		Add, rename and substitute all the necessary files
		in the existing zip archives. No new zip archives
		are created, only the already present zip files are
		modified. No file is deleted. Anyway, some files may
		be overwritten. Any file overwritten is saved in
		the `rom_unknown' or `sample_unknown' directory in
		a zip archive with the same name of the original
		one.

	-d, --del-zip
		Remove any unknown zip or chd archive. Any archive
		removed is saved in the `rom_unknown',
		`sample_unknown' or `disk_unknown' directory.

	-u, --del-unknown
		Remove any unknown file from the existing zip
		archives. Any file removed is saved in the
		`rom_unknown' or `sample_unknown' directory in a
		zip archive with the same name of the original one.
		Only binary files are removed, all the text files
		are keep.

	-g, --del-garbage
		Remove any garbage file from the zip archives. A
		garbage file is an advertising text file added
		generally by a rom site. The files removed are not
		saved.

	-t, --del-text
		Remove any text file from the zip archives. Any
		file removed is saved in the `rom_unknown' or
		`sample_unknown' directory in a zip archive with the
		same name of the original one.

	-R, --rom-std
		Shortcut for the options -rabdug. It does all the
		previous operations on roms except removing text
		files.

	-S, --sample-std
		Shortcut for the options -sabdug. It does all the
		previous operations on samples except removing text
		files.

	-K, --disk-std
		Shortcut for the options -kd. It moves any unknown
		disk file in the disk_unknown directory.

	-n, --print-only
		Don't modify anything, it only shows operations.
		This option prevents any changes made by the
		previous commands. The operations are only printed and
		NOT executed.

	-p, --report
		Write an extensive text report with the list of
		good, bad and missing roms or/and samples. The
		content of any zip archive with missing files is
		printed. You must also specify the -r or/and -s
		options.

	-P, --report-file
		Write a small text report with information on the
		present files, listing incomplete, duplicate or
		missing required zips.

	-f, --filter FILTER
		Apply a specific filter at the rom list. Check the
		FILTERS chapter for a detailed list of filters available.

	-v, --verbose
		Print a more verbose report. The content of any zip
		archive is printed if it contains at least one
		unknow or bad rom file.

Information Options
	The following options are used only to print information.
	These options don't need the configuration file and don't
	access you roms.

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
	are not really decompressed, but the crc values stored in
	the zip archives are used. If a rom has an incorrect crc or
	size, but it has a correct name, it's maintained if
	doesn't exist a valid alternative.

	Sample files are identified only by their names. This
	limits the possible operations. Essentially advscan can
	report only missing samples.

	Garbage files are identified by their size and crc.

	All the others files are identified with this algorithm:

	* If the name is like *.sam, *.wav it's considered an
		unknown binary file.
	* If the name is like *.doc, *.txt, *.nfo,
		*.diz, readme.* it's considered a text file.
	* If the size is a power of 2 it's considered an
		unknown binary file.
	* Otherwise it's considered a text file.

Configuration
	To run advscan you need two files. The rom information
	file and the configuration file.

	The rom information file is the file that contains the
	information of all the roms used by the emulator. It can
	be made with the command:

		:mame -listxml > info.xml

	This file is expected as input of advscan. So, you can use
	this command:

		:advscan [options] < info.xml

	Or combine the two commands together:

		:mame -listxml | advscan [options]

	The configuration file is a text file that describes your
	directories structure. You can use absolute path or
	relative path. Relative path is relative to the current
	directory from where you run advscan.

	In Unix the PATH separator is `:'. In DOS and Windows the PATH
	separator is `;'. The following options are expressed with the
	Unix format.

	=rom PATH:PATH...
		List of paths where the roms are placed. These are
		the zip archives, which are modified and fixed.

	=rom_new PATH
		Single directory where the new created zip archives are
		placed. It's STRONGLY suggested to put this path
		ALSO in the `rom' specification. Otherwise at the
		next run the zip archives are recreated.

	=rom_import PATH:PATH...
		List of directory trees where other roms files are
		placed. These are used for importing rom file missing
		in rompath. These files are only read and never
		modified in any way. It's very useful to insert
		here any rom directories of any other arcade
		emulators. When a new game will be supported the rom
		archive will be made automatically.

	=rom_unknown PATH
		Single directory where unknown rom zip archives will be
		moved. In this directory is inserted any rom file
		removed from the rom zip archives. However, any rom
		file is automatically deleted by advscan if it's
		duplicated in an archive listed on the `rom' or
		`rom_import' options.

	=sample PATH:PATH...
		List of paths where the samples are placed. These
		are the zip archives, which are modified and fixed.

	=sample_unknown PATH
		Single path where unknown sample zip archives will
		be moved. In this directory is inserted any sample
		file removed from the sample zip archives.

	=disk PATH:PATH...
		List of paths where the disks are placed. These
		are the chd archives.

	=disk_unknown PATH
		Single path where unknown disk chd archives will
		be moved.

	If the -c option is not specified the configuration file
	is read from ./advscan.rc.

	The files advscan.rc.linux and advscan.rc.dos are two
	examples of configuration files.

Damaged Files
	If a damaged .zip or .chd file is detected, it's renamed
	with a new name appending the extension ".damaged".
	For .chd file it may happen that the file is a disk image
	with an old format.

	If the rename operation isn't possible the program aborts.

	Please note that the program doesn't execute an
	extensive test to detect damaged files.

Filters
	As default advscan uses all the rom definitions, including also
	unplayable games. If you prefere you can use only a subset
	of the roms defined with the --filter option.

	The filters available are:
		working - Use only working roms. A working rom is a rom
			reported working or a rom needed by another working
			rom. A rom is reported working if it isn't marked
			with the "preliminary" flag in the driver, video or
			sound. This should be the preferred filter which only
			store playable games.
		preliminary - Use only roms which are not in the "working"
			subset.
		working_parent - Use only working parent roms from the
			"working" subset excluding clones. If a parent rom
			is not working but it has a working clone, both
			the parent and the clone are used.
			This is the preferred filter if you want only one
			working game for any parent/clones group.
		working_clone - Use only roms which  are in the "working"
			subset but not in the "working_parent" subset.

	For example, if you want to keep preliminary and working roms in
	different directories, you can setup two different advscan.rc files
	with different directories but sharing the `rom_unknown' dir to allow
	automatic rom moving between the two sets.
	If this is your configuration, to update your romset, you need to
	run advscan three times:

		on working set - To export preliminary roms in the
			rom_unknown dir.
		on preliminary set - To import preliminary roms and export
			working roms in the rom_unknown dir.
		on working set - To import working rom from the
			rom_unknown dir.

	For example:
		advscan -R -c advscan-wrk.rc -f working < advmame.xml > rom_wrk.log
		advscan -R -c advscan-pre.rc -f preliminary < advmame.xml > rom_pre.log
		advscan -R -c advscan-wrk.rc -f working < advmame.xml > rom_wrk.log

Report Format
	The report generated with the -p option contains some text
	tag explained here:

	=rom_good
		A recognized good rom. The rom is recognized by its
		name, crc and size.

	=rom_bad
		A recognized bad rom with an incorrect size or crc.
		The rom is recognized by its name.

	=rom_miss
		A missing rom.

	=disk_bad
		A recognized bad disk with an incorrect hash.
		The disk is recognized by its name.

	=nodump_good
		A fake "NO GOOD DUMP KNOWN" rom. The rom is
		recognized by its name, size and crc.

	=nodump_miss
		A missing "NO GOOD DUMP KNOWN" rom. It's the normal
		condition, a no dump rom must be missing.

	=nodump_bad
		A recognized bad "NO GOOD DUMP KNOWN" rom. The rom is
		recognized by its name.

	=sound_good
		A recognized good sound sample. The sample is
		recognized by its name.

	=sound_miss
		A missing sound sample.

	=text
		An unknown text file.

	=binary
		An unknown binary file.

	=garbage
		A recognized garbage file. A garbage file is an
		advertising text file added generally by a rom
		site. The file is recognized by its name, size and
		crc.

Examples
	For the generic use you need to run advscan with the
	options:

		:advscan -R < info.xml

	This command will fix your rom collection (without removing
	the precious text files).

	To check in advance all the operations that will be done
	you can use the command:

		:advscan -R -n < info.xml

	which only show the operations.

	To only generate an extensive report of your rom set you
	can use the command:

		:advscan -r -p < info.xml > report.txt

	To increase the verbosity of the printed information you
	can add the -v switch.

Copyright
	This file is Copyright (C) 2003, 2004, 2008 Andrea Mazzoleni, Filipe Estima

See Also
	advdiff(1)

