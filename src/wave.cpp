/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * wave
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */

#include "wave.h"

extern Conf G_Conf;


Wave::Wave() : data(NULL), size(0), isLogical(0), isEdited(0) {}


/* ------------------------------------------------------------------ */


Wave::~Wave() {
	clear();
}


/* ------------------------------------------------------------------ */


int Wave::open(const char *f) {
	pathfile = f;
	name   = gBasename(f);
#if defined(__linux__)
	name   = stripExt(name.c_str());  // in linux gBasename doesn't strip
#endif
	fileIn = sf_open(f, SFM_READ, &inHeader);
	if (fileIn == NULL) {
		printf("[wave] unable to read %s\n", f);
		pathfile = "";
		name     = "";
		return 0;
	}
	isLogical = false;
	isEdited  = false;
	return 1;
}


/* ------------------------------------------------------------------ */

/* how to read and write with libsndfile:
 *
 * a frame consists of all items (samples) that belong to the same
 * point in time. So in each frame there are as many items as there
 * are channels.
 *
 * Quindi:
 * 	frame  = [item, item, ...]
 * In pratica:
 *  frame1 = [itemLeft, itemRight]
 * 	frame2 = [itemLeft, itemRight]
 * 	...
 */

int Wave::readData() {

	size = inHeader.frames * inHeader.channels;
	data = (float *) malloc(size * sizeof(float));
	if (data == NULL) {
		puts("[wave] unable to allocate memory");
		return 0;
	}

	if (sf_read_float(fileIn, data, size) != size)
		puts("[wave] warning: incomplete read!");

	sf_close(fileIn);
	return 1;
}


/* ------------------------------------------------------------------ */


int Wave::writeData(const char *f) {

	/* prepare the header for output file */

	outHeader.samplerate = inHeader.samplerate;
	outHeader.channels   = inHeader.channels;
	outHeader.format     = inHeader.format;

	fileOut = sf_open(f, SFM_WRITE, &outHeader);
	if (fileOut == NULL) {
		printf("[wave] unable to open %s for exporting\n", f);
		return 0;
	}

	if (sf_write_float(fileOut, data, size) != size) {
		printf("[wave] error while exporting %s", f);
		return 0;
	}

	isLogical = false;
	isEdited  = false;
	sf_close(fileOut);
	return 1;
}


/* ------------------------------------------------------------------ */


void Wave::clear() {
	if (data != NULL) {
		free(data);
		data     = NULL;
		pathfile = "";
		size     = 0;
	}
}


/* ------------------------------------------------------------------ */


int Wave::allocEmpty(unsigned __size) {

	/* the caller must pass a __size for stereo values */

	size = __size;
	data = (float *) malloc(size * sizeof(float));
	if (data == NULL) {
		puts("[wave] unable to allocate memory");
		return 0;
	}

	memset(data, 0, sizeof(float) * size); /// FIXME - is it useful?

	inHeader.samplerate = G_Conf.samplerate;
	inHeader.channels   = 2;
	inHeader.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT; // wave only

	isLogical = true;
	return 1;
}