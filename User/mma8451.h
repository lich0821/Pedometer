#ifndef __MMA8451_H__
#define __MMA8451_H__

#include <stdbool.h>
#include <stdint.h>

#define MMA8451_STATUS_00      		0x00
#define MMA8451_OUT_X_MSB       	0x01
#define MMA8451_OUT_X_LSB       	0x02
#define MMA8451_OUT_Y_MSB       	0x03
#define MMA8451_OUT_Y_LSB       	0x04
#define MMA8451_OUT_Z_MSB       	0x05
#define MMA8451_OUT_Z_LSB       	0x06
#define MMA8451_F_STATUS        	0x00
#define MMA8451_F_SETUP         	0x09
#define MMA8451_TRIG_CFG        	0x0A
#define MMA8451_SYSMOD          	0x0B
#define MMA8451_INT_SOURCE      	0x0C
#define MMA8451_WHO_AM_I        	0x0D
#define MMA8451_XYZ_DATA_CFG    	0x0E
#define MMA8451_HP_FILTER_CUTOFF	0x0F
#define MMA8451_PL_STATUS       	0x10
#define MMA8451_PL_CFG          	0x11
#define MMA8451_PL_COUNT        	0x12
#define MMA8451_PL_BF_ZCOMP     	0x13
#define MMA8451_PL_P_L_THS_REG  	0x14
#define MMA8451_FF_MT_CFG       	0x15
#define MMA8451_FF_MT_SRC       	0x16
#define MMA8451_FF_MT_THS       	0x17
#define MMA8451_FF_MT_COUNT     	0x18
#define MMA8451_TRANSIENT_CFG   	0x1D
#define MMA8451_TRANSIENT_SRC   	0x1E
#define MMA8451_TRANSIENT_THS   	0x1F
#define MMA8451_TRANSIENT_COUNT 	0x20
#define MMA8451_PULSE_CFG       	0x21
#define MMA8451_PULSE_SRC       	0x22
#define MMA8451_PULSE_THSX      	0x23
#define MMA8451_PULSE_THSY      	0x24
#define MMA8451_PULSE_THSZ      	0x25
#define MMA8451_PULSE_TMLT      	0x26
#define MMA8451_PULSE_LTCY      	0x27
#define MMA8451_PULSE_WIND      	0x28
#define MMA8451_ASLP_COUNT      	0x29
#define MMA8451_CTRL_REG1       	0x2A
#define MMA8451_CTRL_REG2       	0x2B
#define MMA8451_CTRL_REG3       	0x2C
#define MMA8451_CTRL_REG4       	0x2D
#define MMA8451_CTRL_REG5       	0x2E
#define MMA8451_OFF_X           	0x2F
#define MMA8451_OFF_Y           	0x30
#define MMA8451_OFF_Z           	0x31

//Return of MMA8451_WHO_AM_I
#define MMA8451Q_ID           		0x1A

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
}xyzdata_t;

typedef union
{
	uint8_t Byte[2];
	uint16_t Value;
}Data_t;



//#define FULL_SCALE

bool mma8451_init(uint8_t device_address);
bool mma8451_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes);
bool mma8451_register_write(uint8_t register_address, uint8_t value);
void mma8451_get_xyz( float *ax, float *ay, float *az );
xyzdata_t mma8451_get_data(void);
//void mma8451_read_fifo(xyzdata_t accelData[]);
bool mma8451_read_fifo(uint8_t fifoData[]);
xyzdata_t mma8451_deal_fifo(uint8_t fifoData[]);
void mma8451_MD_conf( void );
bool mma8451_FIFO_conf(void);


#endif
