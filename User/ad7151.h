#ifndef __AD7151_H__
#define __AD7151_H__

#include <stdbool.h>
#include <stdint.h>

#define AD7151_STATUS 			0x00
#define AD7151_DATA_HIGH 		0x01
#define AD7151_DATA_LOW 		0x02
#define AD7151_AVERAGE_HIGH 	0x05
#define AD7151_AVERAGE_LOW 		0x06
#define AD7151_SENS_THR_H 		0x09
#define AD7151_SENS_THR_L 		0x0A
#define AD7151_SETUP 			0x0B
#define AD7151_CONFIGURATION 	0x0F
#define AD7151_POWERDOWN_TIMER 	0x10
#define AD7151_CAPDAC 			0x11
#define AD7151_SERIAL_NUM_3 	0x13
#define AD7151_SERIAL_NUM_2 	0x14
#define AD7151_SERIAL_NUM_1 	0x15
#define AD7151_SERIAL_NUM_0 	0x16
#define AD7151_CHIP_ID 			0x17
#define AD7151_RESET			0xBF

//#define FULL_SCALE

bool ad7151_init(uint8_t device_address);
bool ad7151_conf(void);
bool ad7151_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes);
bool ad7151_register_write(uint8_t register_address, uint8_t value);
float ad7151_get_cap( void );


#endif
