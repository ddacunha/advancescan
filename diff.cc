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

#include "portable.h"

#include "gamediff.h"
#include "readinfo.h"
#include "error.h"
#include "portable.h"

#include <iostream>

#include <unistd.h>

using namespace std;

void version() {
	std::cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << std::endl;
}

void usage() {
	version();

	std::cout << "Usage: advdiff INFO1.txt INFO2.txt" << std::endl;
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "hV"

void process(int argc, char* argv[]) {
	game_set_load g0;
	game_set_load g1;
	FILE* f;

	if (argc <= 1) {
		usage();
		return;
	}

	int c = 0;

	opterr = 0; // don't print errors

	while ((c =
#ifdef HAVE_GETOPT_LONG
		getopt_long(argc, argv, OPTIONS, long_options, 0))
#else
		getopt(argc, argv, OPTIONS))
#endif
	!= EOF) {
		switch (c) {
			case 'h' :
				usage();
				return;
			case 'V' :
				version();
				return;
			default: {
				// not optimal code for g++ 2.95.3
				string opt;
				opt = (char)optopt;
				throw error() << "Unknow option `" << opt << "'";
			}
		} 
	}

	if (argc - optind < 2)
		throw error() << "Missing input files";
	if (argc - optind > 2)
		throw error() << "Too many input files";

	string f0 = argv[optind+0];
	string f1 = argv[optind+1];

	f = fopen(f0.c_str(),"r");
	if (!f)
		throw error() << "Failed open of " << f0;
	if (!g0.load(f,true))
		throw error() << "Failed read of " << f0 << " at row " << info_row_get()+1 << " at column " << info_col_get()+1;
	fclose(f);

	f = fopen(f1.c_str(),"r");
	if (!f)
		throw error() << "Failed open of " << f1;
	if (!g1.load(f,true))
		throw error() << "Failed read of " << f1 << " at row " << info_row_get()+1 << " at column " << info_col_get()+1;
	fclose(f);

	game_set::const_iterator i;
	for(i=g1.begin();i!=g1.end();++i) {
		game_set::const_iterator j = g0.find(*i);
		if (j==g0.end() || !include(j->roms_get(),i->roms_get()))
			std::cout << *i << std::endl;
	}
}

int main(int argc, char* argv[]) {

	try {
		process(argc,argv);
	} catch (error& e) {
		cerr << e << endl;
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory" << endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknow error" << endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
