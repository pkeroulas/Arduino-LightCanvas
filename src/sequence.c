/*
 *  Copyright (C) 2014 Patrick Keroulas.
 *  Author: Patrick Keroulas <patkarbo@patkarbo.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//avr
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
// local includes
#include "sequence.h"

// structure:
// * periodic flags
// * offset: offset_type(0x0#-0x7#)|variable_flag(0x80)|delay_factor(0x#0-0x#F)
// * envelop: waveform|amplitude(0-3)
// * speed(<25)

const uint8_t sequence_1[SEQ_SIZE] = {  0, // random fast
    ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 11,
    ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 11,
    ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 11,
    ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 11
};/*
     const uint8_t sequence_1[1+LAYER_N*SEQ_LAYER_SIZE] = {  SEQ_FLAGS_PERIODIC_LAYER,	//random slow
     ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 15,
     ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 15,
     ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 15,
     ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_PULSE|2, 15,
     };*/
const uint8_t sequence_2[SEQ_SIZE] = {  SEQ_FLAGS_GLITCH_PIXEL, // random fast with glitches
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_PULSE|2, 7,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_PULSE|2, 7,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_PULSE|2, 7,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_PULSE|2, 7
};
const uint8_t sequence_3[SEQ_SIZE] = {  SEQ_FLAGS_PERIODIC_PIXEL, // random slopes
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_SLOPE|2, 20,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_SLOPE|2, 20,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_SLOPE|2, 20,
    ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, ENV_WAVEFORM_SLOPE|2, 20,
};
const uint8_t sequence_4[SEQ_SIZE] = {  SEQ_FLAGS_PERIODIC_PIXEL, // falling sky
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PULSE|3, 5,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PULSE|3, 5,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PULSE|3, 5,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PULSE|3, 5,
};
const uint8_t sequence_5[SEQ_SIZE] = {  SEQ_FLAGS_PERIODIC_PIXEL,	// clock wise
    ENV_OFFSET_TYPE_FORWARD|4, ENV_WAVEFORM_EPOLS, 10,
    ENV_OFFSET_TYPE_FORWARD|4, ENV_WAVEFORM_EPOLS, 10,
    ENV_OFFSET_TYPE_FORWARD|4, ENV_WAVEFORM_EPOLS, 10,
    ENV_OFFSET_TYPE_FORWARD|4, ENV_WAVEFORM_EPOLS, 10,
};
const uint8_t sequence_6[SEQ_SIZE] = {  SEQ_FLAGS_PERIODIC_PIXEL,	// left wave
    ENV_OFFSET_TYPE_LEFTWAVE|ENV_OFFSET_FLAG_VARIABLE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_LEFTWAVE|ENV_OFFSET_FLAG_VARIABLE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_LEFTWAVE|ENV_OFFSET_FLAG_VARIABLE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_LEFTWAVE|ENV_OFFSET_FLAG_VARIABLE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
};
const uint8_t sequence_7[SEQ_SIZE] = {  0,	// left wave -> diag wave
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
    ENV_OFFSET_TYPE_DOWNWAVE|5, ENV_WAVEFORM_PLAIN|ENV_INV_MASK, 7,
};
const uint8_t sequence_8[SEQ_SIZE] = {  SEQ_FLAGS_PERIODIC_PIXEL,	// up wave
    ENV_OFFSET_TYPE_UPWAVE|4, ENV_WAVEFORM_PLAIN, 25,
    ENV_OFFSET_TYPE_UPWAVE|4, ENV_WAVEFORM_PLAIN, 25,
    ENV_OFFSET_TYPE_UPWAVE|4, ENV_WAVEFORM_PLAIN, 25,
    ENV_OFFSET_TYPE_UPWAVE|4, ENV_WAVEFORM_PLAIN, 25,
};

// sequence global param
uint8_t * sequence_tab[SEQ_N] = {&sequence_1,&sequence_2,&sequence_3,&sequence_4,&sequence_5,&sequence_6,&sequence_7,&sequence_8};
uint8_t sequence_index;

//sequencer
void sequence_init(void){

}

void sequence_update(uint8_t index){
    if(index >= SEQ_N) {
        printf("Out of range index: %d/%d\n", index, SEQ_N);
        return;
    }
    sequence_index = index;
    printf("Sequence index: %d\n", sequence_index);
    memcpy((uint8_t *) sequence, sequence_tab[sequence_index],1+LAYER_N*SEQ_LAYER_SIZE);
}

void sequence_skip(void){
    if(++sequence_index >= SEQ_N) {
        sequence_index = 0;
    }
    printf("Sequence index: %d\n", sequence_index);
    memcpy((uint8_t *) sequence, sequence_tab[sequence_index],1+LAYER_N*SEQ_LAYER_SIZE);
}
