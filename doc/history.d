Name
	advscan - History For AdvanceSCAN

AdvanceSCAN Version 2.0 2018/01
	) Ignore missing definitions of sampleof. Recent MAMEs have a lot of them.
	) Windows binaries built with MingW 4.9.3 using the MXE cross compiler at
		commit 62bcdbee56e87c81f1faa105b8777a5879d4e2e with targets
		i686-w64-mingw32 and x86_64-w64-mingw32.
	) DOS binaries built with DJGPP 4.8.5 from
		https://github.com/andrewwutw/build-djgpp

AdvanceSCAN Version 1.18 2015/08
	) Updated to new autoconf/git format.
	) Other minor changes.

AdvanceSCAN Version 1.17 2013/02
	) In the report output (--report) don't complain about preliminary BIOS.
	) If a rom contains only not dumped files, don't report it missing if
		no zip is present.
	) Now in git repository.

AdvanceSCAN Version 1.16 2012/11
	) Added support for CHD versions 4 and 5.
	) Fixed the printout of the CHD hash.
	) Fixed samples extension handling.
	) Removed zlib.dll from the Windows distribution. It's no more required.

AdvanceSCAN Version 1.15 2012/11
	) Fixed sampleof recognition.

AdvanceSCAN Version 1.14 2008/12
	) Fixed the recognition of the XML 'inbios' atttribute. It's an
		important fix if you use the 'filter' feature.
	) Fixed support for 64 bits architectures.

AdvanceSCAN Version 1.13 2004/12
	) New rom zips are now created only if at least one unique
		rom is found. Roms shared with other games don't
		trigger the zip creation. For example, galaga88
		and pacmania share the rom ns1-mcu.bin. This
		functionality prevents the creation of the galaga88
		zip from a correct copy of pacmania using only the
		ns1-mcu.bin file.
	) Relaxed a consistency check for the local header in zip files.
		The crc and size entries are allowed to contain the real
		value also if a data descriptor is present.

AdvanceSCAN Version 1.12 2004/11
	) In -listinfo files are now allowed crc with the "0x"
		prefix.

AdvanceSCAN Version 1.11 2004/09
	) Minor changes at the source. Removed the gamediff.* files.
	) Fixed a compilation problem with gcc 3.4.

AdvanceSCAN Version 1.10 2004/08
	) Added support for .chd files. Check the new -K option.
	) The program now automatically renames any damaged .zip
		and .chd file instead of aborting.
	) Updated the included expat library to version 1.95.8.

AdvanceSCAN Version 1.9 2004/07
	) Fixed a Mac OS X crash problem with XML files with
		empty items.
	) Only the XML driver.status entry is now used to recognize
		the game emulation status. Game not marked as
		"preliminary" are considered working games.

AdvanceSCAN Version 1.8 2004/04
	) Added a filter capability able to filter in/out preliminary, parent
		and clone  roms. For example you can now keep separated
		preliminary and working roms.
	) Added support for xml format at advdiff.
	) Updated the expat library at version 1.95.7.

AdvanceSCAN Version 1.7 2003/08
	) Added support for reading XML info file.
	) Updated with automake 1.7.6.

AdvanceSCAN Version 1.6 2003/05
	) Fixed the support for zips with additional data descriptors.
	) Updated with autoconf 2.57 and automake 1.7.4.
	) Added support for the new `nodump' flag in MAME 0.68.
	) Removed the `redump' output because no more required.
		Now MAME 0.68 outputs the correct crc and no more
		the NOT version. No more ambiguity.
	) Some fixes for the gcc 3.3 compiler.

AdvanceSCAN Version 1.5 2003/02
	) Removed the recompression utilities. Now they are in the
		AdvanceCOMP package.

AdvanceSCAN Version 1.4 2002/12
	) Fixed a bug in the advmng utility when it was called with
		more than one file in the command line. The program
		was incorrectly adding a PLTE chunk at rgb images.

AdvanceSCAN Version 1.3 2002/11
	) Fixed a bug in the advscan utility. When a .zip file was completly
		removed the program was aborting with a "Failed stat" message
		because it was trying to reread the just deleted file.
	) Added the support for the transparency tRNS chunk at the
		advpng utility.
	) Improved the garbage detector for zipped roms.
	) Upgraded at the lastest Advance Library.
	) Fixes at the docs. [by Filipe Estima]
	) Minor changes at the autoconf/automake scripts.

AdvanceSCAN Version 1.2 2002/08
	) Added the advpng utility to compress the PNG files.
	) Added the advmng utility to compress the MNG files.
	) Added a Windows version.
	) Other minor fixes.

AdvanceSCAN Version 1.1 2002/06
	) Fixed an infinite loop bug testing some small damaged zips.
	) Removed some warning compiling with gcc 3.1.

AdvanceSCAN Version 1.0 2002/05
	) First public release.
	) Fixed the compression percentage computation on big files.
	) The unknow .zip files are now deleted if they contains only
		empty directories.
	) Added the --pedantic option at the advzip utility. These
		tests are only done if requested.
	) Renamed all the "unknow" options in "unknown".
	) Fixed the reading of configuration file containing the \r char.
	) Documentation fixes.
	) Other minor fixes.

AdvanceSCAN Version 0.6-beta 2002/05
	) Major revision.
	) Renamed AdvanceSCAN.
	) Updated to the last C++ standard.
	) General cleanup of the code.
	) Added the AdvanceDIFF and AdvanceZIP utility.

MAMESCAN Version 0.5 2001/08
	) Removed the options -fix, -cat, -collision.

MAMESCAN Version 0.4 2000/02
	) Added configure for msdos.

MAMESCAN Version 0.3
	) Minor revison.

MAMESCAN Version 0.2
	) Minor revison.

MAMESCAN Version 0.1 1999/07
	) First version.

MAMESCAN Version 0.0 1998/11
	) Posted in the MAME list the new listinfo command.

