/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * pluginHost
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

#ifdef WITH_VST

#include "pluginHost.h"



extern Conf       G_Conf;
extern Mixer      G_Mixer;
extern PluginHost G_PluginHost;
extern unsigned   G_beats;


PluginHost::PluginHost() {

	/* initially we fill vstTimeInfo with trash. Only when the plugin requests
	 * the opcode we load the right infos from G_Mixer. */

	vstTimeInfo.samplePos          = 0.0;
	vstTimeInfo.sampleRate         = 44100;
	vstTimeInfo.nanoSeconds        = 0.0;
	vstTimeInfo.ppqPos             = 0.0;
	vstTimeInfo.tempo              = 120.0;
	vstTimeInfo.barStartPos        = 0.0;
	vstTimeInfo.cycleStartPos      = 0.0;
	vstTimeInfo.cycleEndPos        = 0.0;
	vstTimeInfo.timeSigNumerator   = 4;
	vstTimeInfo.timeSigDenominator = 4;
	vstTimeInfo.smpteOffset        = 0;
	vstTimeInfo.smpteFrameRate     = 1;
	vstTimeInfo.samplesToNextClock = 0;
	vstTimeInfo.flags              = 0;
}


/* ------------------------------------------------------------------ */


PluginHost::~PluginHost() {}


/* ------------------------------------------------------------------ */


int PluginHost::allocBuffers() {

	/** FIXME - ERROR CHECKING! */

	bufferI    = (float **) malloc(2 * sizeof(float*));
	bufferI[0] =  (float *) malloc(G_Conf.buffersize * sizeof(float));
	bufferI[1] =  (float *) malloc(G_Conf.buffersize * sizeof(float));

	bufferO    = (float **) malloc(2 * sizeof(float*));
	bufferO[0] =  (float *) malloc(G_Conf.buffersize * sizeof(float));
	bufferO[1] =  (float *) malloc(G_Conf.buffersize * sizeof(float));

	memset(bufferI[0], 0, sizeof(float) * G_Conf.buffersize);
	memset(bufferI[1], 0, sizeof(float) * G_Conf.buffersize);
	memset(bufferO[0], 0, sizeof(float) * G_Conf.buffersize);
	memset(bufferO[1], 0, sizeof(float) * G_Conf.buffersize);

	printf("[pluginHost] buffers allocated, buffersize = %d\n", G_Conf.buffersize);

	//printOpcodes();

	return 1;
}


/* ------------------------------------------------------------------ */


VstIntPtr VSTCALLBACK PluginHost::HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	return G_PluginHost.gHostCallback(effect, opcode, index, value, ptr, opt);
}


/* ------------------------------------------------------------------ */


VstIntPtr PluginHost::gHostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {

	/* warning: VST headers compiled with DECLARE_VST_DEPRECATED. */

	switch (opcode) {

		/* 0 - Called after a control has changed in the editor and when
		 * the associated parameter should be automated. Index contains the
		 * param, opt the value. Thanks, but we don't need it now. It will
		 * be useful when recording actions from VST (in the future). */

		case audioMasterAutomate:
			return 0;

		/* 1 - host version (2.4) */

		case audioMasterVersion:
			return kVstVersion;

		/* 3 - Give idle time to Host application, e.g. if plug-in editor is
		 * doing mouse tracking in a modal loop. This a is multithread app,
		 * we don't need it. */

		case audioMasterIdle:
			return 0;

		/* 7 - time infos */

		case audioMasterGetTime:
			vstTimeInfo.samplePos          = G_Mixer.actualFrame;
			vstTimeInfo.sampleRate         = G_Conf.samplerate;
			vstTimeInfo.tempo              = G_Mixer.bpm;
			vstTimeInfo.timeSigNumerator   = G_Mixer.beats;
			vstTimeInfo.timeSigDenominator = G_Mixer.bars;
			vstTimeInfo.ppqPos             = G_beats;
			return (VstIntPtr) &vstTimeInfo;

		/* ? - requires a pointer to VstEvents. No vstEvents so far (v0.5.4) */

		case audioMasterProcessEvents:
			return 0;

		/* ? - tells that numInputs/numOutputs are changed. Not supported and
		 * not needed. */

		case audioMasterIOChanged:
			return false;

		case audioMasterSizeWindow:
			printf("[pluginHost] requested opcode 'audioMasterSizeWindow' (%d)\n", audioMasterSizeWindow);
			return 0;

		case audioMasterGetSampleRate:
			printf("[pluginHost] requested opcode 'audioMasterGetSampleRate' (%d)\n", audioMasterGetSampleRate);
			return 0;

		case audioMasterGetBlockSize:
			printf("[pluginHost] requested opcode 'audioMasterGetBlockSize' (%d)\n", audioMasterGetBlockSize);
			return 0;

		case audioMasterGetInputLatency:
			printf("[pluginHost] requested opcode 'audioMasterGetInputLatency' (%d)\n", opcode);
			return 0;

		case audioMasterGetOutputLatency:
			printf("[pluginHost] requested opcode 'audioMasterGetOutputLatency' (%d)\n", opcode);
			return 0;

		/* 23 - wants to know what kind of process is that.
		 * kVstProcessLevelRealtime = currently in audio thread (where
		 * process is called). */

		case audioMasterGetCurrentProcessLevel:
			return kVstProcessLevelRealtime;

		case audioMasterCanDo:
			printf("[pluginHost] requested opcode 'audioMasterCanDo' (%d)\n", opcode);
			return 0;

		case audioMasterGetAutomationState:
			printf("[pluginHost] requested opcode 'audioMasterGetAutomationState' (%d)\n", opcode);
			return 0;

		default:
			printf("[pluginHost] FIXME: host callback called with opcode %d\n", opcode);
			return 0;
	}
}


/* ------------------------------------------------------------------ */


int PluginHost::addPlugin(const char *fname, int stackType, int chan) {

	Plugin *p    = new Plugin();
	bool success = true;

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, chan);

	if (!p->load(fname)) {
		//delete p;
		//return 0;
		success = false;
	}

	/* if the load failed we add a 'dead' plugin into the stack. This is
	 * useful to report a missing plugin. */

	if (!success) {
		pStack->add(p);
		return 0;
	}

	/* otherwise let's try to initialize it. */

	else {

		/* try to init the plugin. If fails, delete it and return error. */

		if (!p->init(&PluginHost::HostCallback)) {
			delete p;
			return 0;
		}

		/* plugin setup */

		p->setup(G_Conf.samplerate, G_Conf.buffersize);

		/* try to add the new plugin until succeed */

		int lockStatus;
		while (true) {
			lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
			if (lockStatus == 0) {
				pStack->add(p);
				pthread_mutex_unlock(&G_Mixer.mutex_plugins);
				break;
			}
		}

		char name[256]; p->getName(name);
		printf("[pluginHost] plugin loaded (%s), stack type = %d, stack size %d\n", name, stackType, pStack->size);

		/* this is suggested. Who knows... */

		p->resume();

		return 1;
	}
}


/* ------------------------------------------------------------------ */


void PluginHost::processStack(float *buffer, int stackType, int chan) {

	gVector <Plugin *> *pStack = getStack(stackType, chan);

	/* empty stack: do nothing */

	if (pStack->size == 0)
		return;

	/* converting buffer from Giada to VST */

	for (int i=0; i<G_Conf.buffersize; i++) {
		bufferI[0][i] = buffer[i*2];
		bufferI[1][i] = buffer[(i*2)+1];
	}

	/* hardcore processing. At the end we swap input and output, so that
	 * the N-th plugin will process the result of the plugin N-1. */

	for (unsigned i=0; i<pStack->size; i++) {
		if (pStack->at(i)->status != 1)
			continue;
		if (pStack->at(i)->bypass)
			continue;
		pStack->at(i)->processAudio(bufferI, bufferO, G_Conf.buffersize);
		bufferI = bufferO;
	}

	/* converting buffer from VST to Giada. A note for the future: if we
	 * overwrite (=) (as we do now) it's SEND, if we add (+) it's INSERT. */

	for (int i=0; i<G_Conf.buffersize; i++) {
		buffer[i*2]     = bufferO[0][i];
		buffer[(i*2)+1] = bufferO[1][i];
	}
}


/* ------------------------------------------------------------------ */


Plugin *PluginHost::getPluginById(int id, int stackType, int chan) {

	gVector <Plugin *> *pStack = getStack(stackType, chan);

	for (unsigned i=0; i<pStack->size; i++) {
		if (pStack->at(i)->getId() == id)
			return pStack->at(i);
	}
	return NULL;
}


/* ------------------------------------------------------------------ */


Plugin *PluginHost::getPluginByIndex(int index, int stackType, int chan) {
	gVector <Plugin *> *pStack = getStack(stackType, chan);
	return pStack->at(index);
}


/* ------------------------------------------------------------------ */


void PluginHost::freeStack(int stackType, int chan) {

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, chan);

	if (pStack->size == 0)
		return;

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
		if (lockStatus == 0) {
			for (unsigned i=0; i<pStack->size; i++)
				delete pStack->at(i);
			pStack->clear();
			pthread_mutex_unlock(&G_Mixer.mutex_plugins);
			break;
		}
	}

}


/* ------------------------------------------------------------------ */


void PluginHost::freeAllStacks() {
	freeStack(PluginHost::MASTER_OUT);
	for (int i=0; i<MAX_NUM_CHAN; i++)
		freeStack(PluginHost::CHANNEL, i);
}


/* ------------------------------------------------------------------ */


void PluginHost::freePlugin(int id, int stackType, int chan) {

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, chan);

	/* try to delete the plugin until succeed. G_Mixer has priority. */

	for (unsigned i=0; i<pStack->size; i++)
		if (pStack->at(i)->getId() == id) {
			int lockStatus;
			while (true) {
				lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
				if (lockStatus == 0) {
					delete pStack->at(i);
					pStack->del(i);
					pthread_mutex_unlock(&G_Mixer.mutex_plugins);
					printf("[pluginHost] plugin id=%d removed\n", id);
					return;
				}
				//else
					//puts("[pluginHost] waiting for mutex...");
			}
		}
	printf("[pluginHost] plugin id=%d not found\n", id);
}


/* ------------------------------------------------------------------ */


void PluginHost::swapPlugin(unsigned indexA, unsigned indexB, int stackType, int chan) {

	gVector <Plugin *> *pStack = getStack(stackType, chan);

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
		if (lockStatus == 0) {
			pStack->swap(indexA, indexB);
			pthread_mutex_unlock(&G_Mixer.mutex_plugins);
			printf("[pluginHost] plugin at index %d and %d swapped\n", indexA, indexB);
			return;
		}
		//else
			//puts("[pluginHost] waiting for mutex...");
	}
}


/* ------------------------------------------------------------------ */


int PluginHost::getPluginIndex(int id, int stackType, int chan) {

	gVector <Plugin *> *pStack = getStack(stackType, chan);

	for (unsigned i=0; i<pStack->size; i++)
		if (pStack->at(i)->getId() == id)
			return i;
	return -1;
}


/* ------------------------------------------------------------------ */


gVector <Plugin *> *PluginHost::getStack(int stackType, int chan) {
	switch(stackType) {
		case MASTER_OUT:
			return &masterOut;
		case MASTER_IN:
			return &masterIn;
		case CHANNEL:
			return &channel[chan];
		default:
			return NULL;
	}
}


/* ------------------------------------------------------------------ */


unsigned PluginHost::countPlugins(int stackType, int chan) {
	gVector <Plugin *> *pStack = getStack(stackType, chan);
	return pStack->size;
}


/* ------------------------------------------------------------------ */


void PluginHost::printOpcodes() {
	puts("[pluginHost] testing opcodes...");
	printf("  opcode 'audioMasterAutomate'  = %d\n", audioMasterAutomate);
	printf("  opcode 'audioMasterVersion'   = %d\n", audioMasterVersion);
	printf("  opcode 'audioMasterCurrentId' = %d\n", audioMasterCurrentId);
	printf("  opcode 'audioMasterIdle'      = %d\n", audioMasterIdle);
	printf("  opcode 'audioMasterGetTime'   = %d\n", audioMasterGetTime);
}

#endif // #ifdef WITH_VST