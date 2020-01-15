/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FMDISPLAY_H_
#define FMDISPLAY_H_


#include "SynthStateAware.h"
#include "LiquidCrystal.h"
#include "Menu.h"
#include "VisualInfo.h"
#include "Storage.h"
#include "Synth.h"



class FMDisplay : public SynthParamListener, public SynthMenuListener, public SynthStateAware, public VisualInfo {
public:
	FMDisplay();
	~FMDisplay();
	void init(LiquidCrystal* lcd, Storage* storage);
	void customCharsInit();

	void updateEncoderValue(int row, int encoder, ParameterDisplay* param, float newValue);
	void updateEncoderName(int row, int encoder);
	void printValueWithSpace(int value);
    void printFloatWithSpace(float value);
    void printScalaFrequency(float value);
	bool shouldThisValueShowUp(int row, int encoder);
	void updateEncoderValue(int refreshStatus);
	void updateCCValue(int refreshStatus);
	void newTimbre(int timbre);

	// VisualInfo
	void midiClock(bool show);
	void noteOn(int timbre, bool show);
	void tempoClick();
	void audioClipping(void);


	static int getLength(const char *str) {
		int length = 0;
		for (const char *c = str; *c != '\0'; c++) {
			length++;
		}
		return length;
	}

	int getRowNumberToDiplay(int row) {
		if (row <= ROW_ENGINE_LAST) {
			return row - ROW_ENGINE_FIRST +1;
		} else if (row <= ROW_OSC_LAST) {
            return row - ROW_OSC_FIRST +1;
        } else if (row <= ROW_ENV_LAST) {
            return ((row - ROW_ENV_FIRST) >> 1) + 1;
        } else if (row <= ROW_MATRIX_LAST) {
            return row - ROW_MATRIX_FIRST +1;
        } else if (row <= ROW_LFOOSC3) {
            return row - ROW_LFO_FIRST +1;
        } else if (row <= ROW_LFOENV2) {
            return row - ROW_LFOENV1 +1;
        } else if (row <= ROW_LFOSEQ2) {
            return row - ROW_LFOSEQ1 +1;
        }
		return 0;
	}

	bool needRefresh() { return refreshStatus != 0; }

	void refreshAllScreenByStep();
	void displayPreset();
	void displayInstrumentNumber();

    void checkPresetModified(int timbre) {
        if (!presetModifed[timbre]) {
            presetModifed[timbre] = true;
            if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT && timbre == currentTimbre) {
            	int length = getLength(this->synthState->params->presetName);
                lcd->setCursor(3 + length,0);
                lcd->print('*');
            }
        }
    }

    int rowInc(MenuState menuState);
    void newSynthMode(FullState* fullState) ;
    void newMenuState(FullState* fullState) ;
    void newMenuSelect(FullState* fullState);
	void menuBack(enum MenuState oldMenutState, FullState* fullState);
	void eraseRow(int row);
	void displayBankSelect(int bankNumber, bool usable, const char* name);
	void displayPatchSelect(int presetNumber, const char* name);

    void newParamValueFromExternal(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue);
    void newParamValue(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue);
    void newcurrentRow(int timbre, int newcurrentRow);
    void updateStepSequencer(int currentRow, int encoder, int oldValue, int newValue);
    void updateArpPattern(int currentRow, int encoder, int oldValue, int newValue);
    void newPresetName(int timbre);

    void showAlgo() {
    	displayAlgo(this->synthState->params->engine1.algo);
    }

    void showIMInformation() {
        displayIMInformation(this->synthState->params->engine1.algo);
    }

    void beforeNewParamsLoad(int timbre) { };
    void afterNewParamsLoad(int timbre);


    void afterNewComboLoad() {
        for (int k=0; k<4; k++) {
            presetModifed[k] = false;
        }
    }

    // Overide SynthParamListener
    void playNote(int timbre, char note, char velocity) {
    	if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT) {
			lcd->setCursor(0,0);
    	} else {
			lcd->setCursor(0,3);
    	}
    	lcd->print((char)7);
    }
    void stopNote(int timbre, char note) {
    	if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT) {
    		lcd->setCursor(0,0);
    		lcd->print('I');
    	} else {
    		lcd->setCursor(0,3);
    		lcd->print(' ');
    	}
    }

    void setRefreshStatus(int refreshStatus) {
    	this->refreshStatus = refreshStatus;
    }

    void setSynth(Synth *psynth) {
    	this->psynth = psynth;
    }

    bool wakeUpFromScreenSaver();
    void displayAlgo(int algo);
    void displayIMInformation(int algo);

private:
	LiquidCrystal* lcd;
	Storage* storage;
	int refreshStatus;
	int displayedRow;
	Synth *psynth;

	int menuRow;
	// Local value preset modified to know whether it's currently showing up
	bool presetModifed[4];
	int currentTimbre;
	// Midi info
	int noteOnCounter[4];
	int algoCounter;
	bool algoCounterForIMInformation;
	unsigned screenSaveTimer;
	bool screenSaverMode;
	int clipCounter;
};

#endif /* FMDISPLAY_H_ */
