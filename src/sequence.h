#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

/*-----------------------------------------------------------------------*/
// General
#define SEQ_RUNMODE_NORMAL  	0
#define SEQ_RUNMODE_SEQUENTIAL	1
#define SEQ_RUNMODE_TOGGLE	    1
#define SEQ_N 			8
#define SEQ_N 			8
#define LAYER_N			4
#define PIXEL_N			8

/*-----------------------------------------------------------------------*/
// Offset

#define ENV_OFFSET_FLAG_VARIABLE	0x80
#define ENV_OFFSET_TYPE_MASK		0x70
#define ENV_OFFSET_SCALE_MASK		0x0F
//#define ENV_OFFSET_TYPE_NONE		0x00
#define ENV_OFFSET_TYPE_CONSTANT	0x00
#define ENV_OFFSET_TYPE_FORWARD		0x10
#define ENV_OFFSET_TYPE_BACKWARD	0x20
#define ENV_OFFSET_TYPE_RAND		0x30
#define ENV_OFFSET_TYPE_UPWAVE		0x40	// use low scale value
#define ENV_OFFSET_TYPE_DOWNWAVE	0x50	// use low scale value
#define ENV_OFFSET_TYPE_RIGHTWAVE	0x60
#define ENV_OFFSET_TYPE_LEFTWAVE	0x70

#define ENV_AMP_MASK			0x0F
#define ENV_INV_MASK			0x80
#define ENV_WAVEFORM_MASK		0x70
#define ENV_WAVEFORM_CONTANT	0x00
#define ENV_WAVEFORM_SQUARE		0x10
#define ENV_WAVEFORM_PLAIN		0x20
#define ENV_WAVEFORM_STROBE		0x30
#define ENV_WAVEFORM_EBORST		0x40
#define ENV_WAVEFORM_SLOPE		0x50
#define ENV_WAVEFORM_EPOLS		0x60
#define ENV_WAVEFORM_PULSE		0x70

/*-----------------------------------------------------------------------*/
// Sequence

#define SEQ_LAYER_SIZE 				3
#define SEQ_SIZE 				    1+LAYER_N*SEQ_LAYER_SIZE
#define SEQ_FLAGS_GLITCH_PIXEL   0x02
#define SEQ_FLAGS_PERIODIC_PIXEL 0x01
#define SEQ_INDEX_FLAG				0

// ---------------------------------- IDEAS
/*
ENV_OFFSET_TYPE_RAND|15, ENV_WAVEFORM_STROBE|3, 2,
ENV_OFFSET_TYPE_RAND|ENV_OFFSET_FLAG_VARIABLE|15, EsNV_WAVEFORM_SLOPE, 20,
ENV_OFFSET_TYPE_FORWARD|5, ENV_WAVEFORM_SQUARE, 13, 
ENV_OFFSET_TYPE_FORWARD|4, ENV_WAVEFORM_STROBE|ENV_INV_MASK|2, 
1,ENV_OFFSET_TYPE_NONE, ENV_WAVEFORM_CONTANT|3, 1,
};*/

/*-----------------------------------------------------------------------*/
// Prototype
void sequence_init(void);
void sequence_update(uint8_t index);
void sequence_skip(void);

/*-----------------------------------------------------------------------*/
//  global var
uint8_t sequence_runmode;
uint8_t sequence[SEQ_SIZE];
// for periodic pixels
uint8_t sequence_pixel_done_counter;
uint8_t sequence_pixel_cycle_counter;




#endif // __SEQUENCE_H__