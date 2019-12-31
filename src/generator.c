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
#include <math.h>
#include <stdio.h>

//local
#include "generator.h"
#include "sequence.h"
#include "display.h"
#include "debug.h"

/*-----------------------------------------------------------------------*/
// offset for factories
const uint8_t horizontal_pos_tab[LAYER_N*PIXEL_N] = {2,3,5,6,1,2,3,3,4,5,6,1,2,3,4,5,6,3,4,6,1,2,4,2,2,3,4,6,0,0,0,0};
const uint8_t vertical_pos_tab  [LAYER_N*PIXEL_N] = {8,8,8,8,6,5,6,7,7,7,7,4,4,5,5,5,6,4,4,3,3,3,3,2,1,1,2,2,0,0,0,0};

/*-----------------------------------------------------------------------*/
// ENVELOPPE
#define ENV_TYPE_LAYER		1
#define ENV_TYPE_PIXEL		2

const uint8_t env_offset_scale_tab[4] = {5,10,20,32};	// max<32 because *8 position * step

// waveforms:
//fadein, sustain, fadeout, delay  [0-10]
const uint8_t env_waveform_constant	[4] = {0,10,0,0};
const uint8_t env_waveform_square	[4] = {0,10,0,10};
const uint8_t env_waveform_plain	[4] = {2,10,4,2};
const uint8_t env_waveform_strobe	[4] = {0,1,0,10};
const uint8_t env_waveform_eborst	[4] = {0,10,0,1};
const uint8_t env_waveform_slope	[4] = {10,0,0,2};
const uint8_t env_waveform_epols	[4] = {0,0,10,2};
const uint8_t env_waveform_pulse	[4] = {10,0,10,0};

typedef struct {
    uint8_t min;
    uint8_t max;
    uint8_t offset_type;
    uint8_t step;
    uint8_t offset;
    uint8_t fade_in;
    uint8_t sustain;
    uint8_t fade_out;
    uint8_t delay;
    uint16_t duration;
    uint16_t index;
} Enveloppe_t;

/*-----------------------------------------------------------------------*/
// pixel

#define DUMMY			0
#define PIXEL_VALUE_MIN 1
#define PIXEL_VALUE_MAX RANGE-1
#define PIXEL_VALUE_END_OF_CYCLE 0xFF

typedef struct {
    Enveloppe_t enveloppe;
} Pixel_t;

/*-----------------------------------------------------------------------*/
// Layer

typedef struct {
    //Enveloppe_t enveloppe;
    Pixel_t pixel_tab[PIXEL_N];
} Layer_t;

Layer_t layer_0;
Layer_t layer_1;
Layer_t layer_2;
Layer_t layer_3;
Layer_t * layer_tab[LAYER_N] = {&layer_0, &layer_1, &layer_2, &layer_3};

/*-----------------------------------------------------------------------*/
// protypes
void enveloppe_update(void);
void enveloppe_compensate(void);
void enveloppe_load(uint8_t index, Enveloppe_t *env, uint8_t *env_param_tab);
uint8_t enveloppe_run(Enveloppe_t *env);
uint8_t pixel_interpolate(uint8_t start, uint8_t end, uint8_t remaining, uint8_t total);

/*-----------------------------------------------------------------------*/
// definitions

FILE debug_stdout = FDEV_SETUP_STREAM(debug_putc, NULL, _FDEV_SETUP_WRITE);

void generator_init(void){
    stdout = &debug_stdout;
    // todo restore sequence_index + 1  from eeprom
    // save current animation in eeprom and restore the next one on boot

    display_init();
    display_start();
}

void generator_update(void){
    enveloppe_update();
}


void generator_run(void){
    static uint8_t value_tab[PIXEL_N*LAYER_N];
    uint8_t value;

    // for each layer (object)
    for(uint8_t l=0;l<LAYER_N;l++){

        // for each pixel (cell)
        for(uint8_t p=0;p<PIXEL_N;p++){					
            value = enveloppe_run(&(layer_tab[l]->pixel_tab[p].enveloppe));
            if((value != PIXEL_VALUE_END_OF_CYCLE) || (sequence[SEQ_INDEX_FLAG] & SEQ_FLAGS_GLITCH_PIXEL))
                value_tab[l*PIXEL_N+p] = value;
            // else keep old value

            if(layer_tab[l]->pixel_tab[p].enveloppe.index == 0){
                sequence_pixel_done_counter++;
                if(layer_tab[l]->pixel_tab[p].enveloppe.offset_type & ENV_OFFSET_FLAG_VARIABLE)
                    enveloppe_load(l*PIXEL_N+p,&(layer_tab[l]->pixel_tab[p].enveloppe), &sequence[1+l*SEQ_LAYER_SIZE]);
                else{
                    //layer_tab[l]->pixel_tab[p].enveloppe.offset = 0;
                }
            }
        }
    }

    // update pixel env if needed
    if(sequence_pixel_done_counter >= LAYER_N*PIXEL_N){
        sequence_pixel_done_counter = 0;
        if(++sequence_pixel_cycle_counter < 10){
            sequence_pixel_cycle_counter = 0;
        }
    }

    // debug	
    /*printf("a[%d]:%d - ",layer_tab[0]->enveloppe.index,amplitude_tab[0]);
      printf("v[%d]:%d - ",layer_tab[0]->pixel_tab[0].enveloppe.index,value_tab[0]);
      printf(">> %d\n",(amplitude_tab[0]*value_tab[0])/RANGE);*/

    // sends the data to the drivers
    display_run(&value_tab[0], LAYER_N*PIXEL_N);
}

/*--------------------------------------------------------------------------------*/
// ENVELOPPES

void enveloppe_update(void){
    //load
    for(uint8_t l=0;l<LAYER_N;l++)
        for(uint8_t p=0;p<PIXEL_N;p++)
            enveloppe_load(l*PIXEL_N+p,&(layer_tab[l]->pixel_tab[p].enveloppe), &sequence[1+l*SEQ_LAYER_SIZE]);

    //compensate == calculate a delay after the enveloppe so that the whole sequence stay periodic
    if(sequence[SEQ_INDEX_FLAG] & SEQ_FLAGS_PERIODIC_PIXEL)
        enveloppe_compensate();

    // print the PIXEL_N first pixel
    for(uint8_t l=0;l<1;l++)
        for(uint8_t p=0;p<PIXEL_N;p++)
            printf("%2d=%3d,%3d,%3d\n",l*8+p,layer_tab[l]->pixel_tab[p].enveloppe.offset,layer_tab[l]->pixel_tab[p].enveloppe.fade_in,layer_tab[l]->pixel_tab[p].enveloppe.sustain);

}

void enveloppe_compensate(void){
    uint16_t maximum_duration = 0;

    for(uint8_t l=0;l<LAYER_N;l++){
        for(uint8_t p=0;p<PIXEL_N;p++){
            //saved the maximum_duration
            if(layer_tab[l]->pixel_tab[p].enveloppe.duration > maximum_duration)
                maximum_duration = layer_tab[l]->pixel_tab[p].enveloppe.duration;
        }
    }
    for(uint8_t l=0;l<LAYER_N;l++)
        for(uint8_t p=0;p<PIXEL_N;p++)
            layer_tab[l]->pixel_tab[p].enveloppe.delay += (uint8_t)(maximum_duration - layer_tab[l]->pixel_tab[p].enveloppe.duration);
}

void enveloppe_load(uint8_t position, Enveloppe_t *env, uint8_t *env_param_tab){

    uint8_t env_waveform[4];
    switch(env_param_tab[1]&ENV_WAVEFORM_MASK){
        case ENV_WAVEFORM_CONTANT:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_constant, 4); break;			
        case ENV_WAVEFORM_SQUARE:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_square, 4); break;			
        case ENV_WAVEFORM_PLAIN:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_plain, 4); break;			
        case ENV_WAVEFORM_STROBE:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_strobe, 4); break;			
        case ENV_WAVEFORM_EBORST:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_eborst, 4); break;			
        case ENV_WAVEFORM_SLOPE:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_slope, 4); break;			
        case ENV_WAVEFORM_EPOLS:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_epols, 4); break;			
        case ENV_WAVEFORM_PULSE:
            memcpy((uint8_t *) env_waveform,(uint8_t *) env_waveform_pulse, 4); break;			
    }	

    // waveform amplitudes
    switch(env_param_tab[1]&ENV_AMP_MASK){
        case 0:
            env->min = 0; env->max = PIXEL_VALUE_MAX; break;			
        case 1:
            env->min = (PIXEL_VALUE_MAX>>2); env->max = PIXEL_VALUE_MAX; break;			
        case 2:
            env->min = (PIXEL_VALUE_MAX>>1); env->max = PIXEL_VALUE_MAX; break;			
        case 3:
            env->min = PIXEL_VALUE_MAX-(PIXEL_VALUE_MAX>>3); env->max = PIXEL_VALUE_MAX; break;			
    }	
    if(env_param_tab[1]&ENV_INV_MASK){
        uint8_t temp = env->min;
        env->min = env->max;
        env->max = temp;
    }

    // waveform timings
    env->step = env_param_tab[2];
    env->fade_in = env_waveform[0] * env->step;
    env->sustain = env_waveform[1] * env->step;
    env->fade_out = env_waveform[2] * env->step;
    env->delay = env_waveform[3] * env->step;	// will be overwritten if periodic flag	
    env->index = 0;

    // calculte offset between envs
    uint8_t position_N = PIXEL_N;
    uint8_t env_offset_scale = ((env_param_tab[0] & ENV_OFFSET_SCALE_MASK) << 4) / position_N;// <<4 because ENV_OFFSET_SCALE_MASK==15 and / by max number of env

    switch(env_param_tab[0] & ENV_OFFSET_TYPE_MASK){
        //case ENV_OFFSET_TYPE_NONE:
        //	env->offset = 0; break;			
        case ENV_OFFSET_TYPE_CONSTANT:
            env->offset = env_offset_scale; break;			
        case ENV_OFFSET_TYPE_FORWARD:
            env->offset = (position%position_N) * env_offset_scale; break;
        case ENV_OFFSET_TYPE_BACKWARD:
            env->offset = (position_N - (position%position_N)) * env_offset_scale; break;
        case ENV_OFFSET_TYPE_UPWAVE:
            env->offset = vertical_pos_tab[position] * env_offset_scale; break;
        case ENV_OFFSET_TYPE_DOWNWAVE:
            env->offset = (PIXEL_N*LAYER_N - vertical_pos_tab[position]) * env_offset_scale; break;
        case ENV_OFFSET_TYPE_RAND:					
            env->offset = (rand() % env_offset_scale); break;
        case ENV_OFFSET_TYPE_RIGHTWAVE:
            env->offset = (PIXEL_N*LAYER_N -horizontal_pos_tab[position]) * env_offset_scale; break;	// special case
        case ENV_OFFSET_TYPE_LEFTWAVE:
            env->offset = horizontal_pos_tab[position] * env_offset_scale; break;	// special case
        default:
            env->offset = 3; break;
    }	

    // special case :do nothing when offset == 0xff
    if(env->offset==0xff){
        env->offset = 0;
        if(env->max > env->min) env->min = env->max;
        else env->max = env->min;
    }
    else{
    }
    //calculate duration
    env->duration = env->offset+ env->fade_in + env->sustain + env->fade_out;
}


uint8_t enveloppe_run(Enveloppe_t * env){
    uint8_t result;
    uint8_t remaining;

    // offset
    if(env->index < (uint16_t)(env->offset))
        result = env->min;

    // fade in
    else if(env->index < (uint16_t)(env->offset + env->fade_in)){
        remaining = env->offset + env->fade_in - env->index;
        result = pixel_interpolate(env->min,env->max,remaining, env->fade_in);
    }
    // sustain
    else if(env->index < (uint16_t)(env->offset + env->fade_in + env->sustain))
        result = env->max;

    // fade_out
    else if(env->index < (uint16_t)(env->offset + env->fade_in + env->sustain + env->fade_out)){
        remaining = env->offset + env->fade_in + env->sustain + env->fade_out - env->index;
        result = pixel_interpolate(env->max,env->min,remaining, env->fade_out);
    }
    // delay
    else if(env->index < (uint16_t)(env->offset + env->fade_in + env->sustain + env->fade_out + env->delay))
        result = env->min;

    // end of cycle
    else{
        env->index = 0;
        return PIXEL_VALUE_END_OF_CYCLE;
    }
    env->index++;

    return result;
}


uint8_t pixel_interpolate(uint8_t start, uint8_t end, uint8_t remaining, uint8_t total) {
    /*return (((signed long)end > (signed long)start) ?
      (signed long)end - ((signed long)end - (signed long)start)*(signed long)remaining/(signed long)total :
      (signed long)end + ((signed long)start - (signed long)end)*(signed long)remaining/(signed long)total);
      */
    return ((end > start) ?
            (unsigned long)end - ((unsigned long)end - (unsigned long)start)*(unsigned long)remaining/(unsigned long)total :
            (unsigned long)end + ((unsigned long)start - (unsigned long)end)*(unsigned long)remaining/(unsigned long)total);
}

