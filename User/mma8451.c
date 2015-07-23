#include "mma8451.h"
#include "twi_master.h"
#include "stdio.h"
#include "nrf_delay.h"
#include "pca10001.h"

static uint8_t m_device_address; //!< Device address in bits [7:1]
extern unsigned char reg_value[0x31];

bool mma8451_verify_product_id(void)
{
	uint8_t who_am_i = 0x00;

	if (mma8451_register_read(MMA8451_WHO_AM_I, &who_am_i, 1))
	{
		if (who_am_i != MMA8451Q_ID)
		{
			//printf("Failed:%X\n", who_am_i);
			return false;
		}
		else
		{
			//printf("Success:%X\n", who_am_i);
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool mma8451_init(uint8_t device_address)
{
	bool transfer_succeeded = true;
//	unsigned char i;

	nrf_gpio_cfg_output(MMA8451_EN);
	nrf_gpio_pin_set(MMA8451_EN);	
	
	m_device_address = (uint8_t)(device_address << 1);

	// Active the MMA8451
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG2, 0x40);
//	nrf_delay_ms(10);
//	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG1, 0x21);	//ODR 50Hz
//	nrf_delay_ms(10);
//	transfer_succeeded &= mma8451_register_write(MMA8451_F_SETUP, 	0x4A);	//Circular mode, 10sa
//	nrf_delay_ms(10);
//	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG4, 0x40);	//Enable FIFO Interrupt
//	nrf_delay_ms(10);
//	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG5, 0x04);	//Route the FIFO to INT1
//	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG2, 0x04);
	if( transfer_succeeded )
		printf("Init Success\n");
	
	// Read and verify product ID
	transfer_succeeded &= mma8451_verify_product_id();
//	for(i=0;i<0x32;i++)
//	{
//		mma8451_register_read(i, &reg_value[i], 1);
//		printf("Reg%02X: %02X\n", i, reg_value[i]);
//	}
	return transfer_succeeded;	
}

bool mma8451_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes)
{
	bool transfer_succeeded;
	transfer_succeeded = twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
	transfer_succeeded &= twi_master_transfer(m_device_address|TWI_READ_BIT, destination, number_of_bytes, TWI_ISSUE_STOP);
	return transfer_succeeded;	
}

bool mma8451_register_write(uint8_t register_address, uint8_t value)
{	
	uint8_t w2_data[2];

	w2_data[0] = register_address;
	w2_data[1] = value;
	return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);	  
}

void mma8451_get_xyz( float *ax, float *ay, float *az )
{
	signed short int x, y, z;
	unsigned char accel_value[6]={0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
	if( mma8451_register_read(MMA8451_OUT_X_MSB, &accel_value[0], 6) )
	{
		x = (accel_value[0] << 6) + (accel_value[1] >> 2);
		y = (accel_value[2] << 6) + (accel_value[3] >> 2);
		z = (accel_value[4] << 6) + (accel_value[5] >> 2);
//		printf("\n%X\t%X\t%X\n", x, y, z);
	}
	x = (x < 0x2000) ? x : -(~(x|0xC000)+1);
	y = (y < 0x2000) ? y : -(~(y|0xC000)+1);
	z = (z < 0x2000) ? z : -(~(z|0xC000)+1);
//	printf("%d\t%d\t%d\n", x, y, z);
	
	*ax = x/4096.0;
	*ay = y/4096.0;
	*az = z/4096.0;
//	printf("%f\t%f\t%f\n\n", *ax, *ay, *az);	
}

xyzdata_t mma8451_get_data(void)
{
	xyzdata_t accelValue;
	//signed short int x, y, z;
	//unsigned char accel_value[6]={0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
	unsigned char accel_value[6];
	if( mma8451_register_read(MMA8451_OUT_X_MSB, &accel_value[0], 6) )
	{
		accelValue.x = (accel_value[0] << 6) + (accel_value[1] >> 2);
		accelValue.y = (accel_value[2] << 6) + (accel_value[3] >> 2);
		accelValue.z = (accel_value[4] << 6) + (accel_value[5] >> 2);
		//printf("\n%X\t%X\t%X\n", accelValue.x, accelValue.y, accelValue.z);
	}
	accelValue.x = (accelValue.x < 0x2000) ? accelValue.x : -(~(accelValue.x|0xC000)+1);
	accelValue.y = (accelValue.y < 0x2000) ? accelValue.y : -(~(accelValue.y|0xC000)+1);
	accelValue.z = (accelValue.z < 0x2000) ? accelValue.z : -(~(accelValue.z|0xC000)+1);
	//printf("%d\t%d\t%d\n", accelValue.x, accelValue.y, accelValue.z);
	return accelValue;
}

bool mma8451_read_fifo(uint8_t fifoData[])
{
	if( mma8451_register_read(MMA8451_OUT_X_MSB, &fifoData[0], 60) )
		return true;
	return false;
}

xyzdata_t mma8451_deal_fifo(uint8_t fifoData[])
{
	uint8_t i, j;
	int16_t x, y, z;
	int32_t x0, y0, z0;
	xyzdata_t accelValue;

	x0 = y0 = z0 = 0;
	for(i=0;i<8;i++)
	{
		j = 6*i;
		x = (fifoData[j] << 6)		+ (fifoData[j+1] >> 2);
		y = (fifoData[j+2] << 6)	+ (fifoData[j+3] >> 2);
		z = (fifoData[j+4] << 6)	+ (fifoData[j+5] >> 2);	

		x = (x < 0x2000) ? x : -(~(x|0xC000)+1);
		y = (y < 0x2000) ? y : -(~(y|0xC000)+1);
		z = (z < 0x2000) ? z : -(~(z|0xC000)+1);
		
		x0 += x;
		y0 += y;
		z0 += z;
	}
	accelValue.x = x0 >> 3;
	accelValue.y = y0 >> 3;
	accelValue.z = z0 >> 3;

	return accelValue;
}

//void mma8451_read_fifo(xyzdata_t accelData[])
//{
//	uint8_t i, j;
//	if( mma8451_register_read(MMA8451_OUT_X_MSB, &reg_value[0], 60) )
//	{
//		for(i=0;i<10;i++)
//		{
//			j = 6*i;
//			accelData[i].x = (reg_value[j] << 6)	+ (reg_value[j+1] >> 2);
//			accelData[i].y = (reg_value[j+2] << 6)	+ (reg_value[j+3] >> 2);
//			accelData[i].z = (reg_value[j+4] << 6)	+ (reg_value[j+5] >> 2);
//			//printf("%d\t", reg_value[i]);
//			accelData[i].x = (accelData[i].x < 0x2000) ? accelData[i].x : -(~(accelData[i].x|0xC000)+1);
//			accelData[i].y = (accelData[i].y < 0x2000) ? accelData[i].y : -(~(accelData[i].y|0xC000)+1);
//			accelData[i].z = (accelData[i].z < 0x2000) ? accelData[i].z : -(~(accelData[i].z|0xC000)+1);
//		}
//	}	
//}


void mma8451_MD_conf( void )
{
	unsigned char CTRL_REG1_data;
	bool transfer_succeeded = true;
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG1, 0x18);	//Set the device in 100Hz ODR, Standby
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_FF_MT_CFG, 0xD8);	//OAE=1, enabling X, Y, Z and the latch
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_FF_MT_THS, 0x0A);	//Threshold:0.063g*0x0A
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_FF_MT_COUNT, 0x0A);	//Set the debounce counter:0x0A=100ms/10ms(steps)
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG4, 0x04);	//Enable Motion Interrupt
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG5, 0x04);	//Route the Motion Interrupt Function to INT1 Hardware pin
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_read(MMA8451_CTRL_REG1, &CTRL_REG1_data, 1);
	CTRL_REG1_data |= 0x01;
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG1, CTRL_REG1_data);
	if( transfer_succeeded )
		printf("MD Success\n");	
}

bool mma8451_FIFO_conf(void)
{
	unsigned char CTRL_REG1_data;
	bool transfer_succeeded = true;
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG1, 0x20);	//Set the device in 50Hz ODR, Standby
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_F_SETUP, 	0x8A);	//Circular mode, 10sa
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG4, 0x40);	//Enable FIFO Interrupt
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG5, 0x40);	//Route the FIFO to INT1
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_read(MMA8451_CTRL_REG1, &CTRL_REG1_data, 1);
	CTRL_REG1_data |= 0x01;
	nrf_delay_ms(10);
	transfer_succeeded &= mma8451_register_write(MMA8451_CTRL_REG1, CTRL_REG1_data);
	if( transfer_succeeded )
		printf("FIFO Success\n");
	return transfer_succeeded;
}
	
//void isr_KBI (void)
//{
//	//clear the interrupt flag
//	CLEAR_KBI_INTERRUPT;
//	//Determine source of interrupt by reading the system interrupt
//	IntSourceSystem=IIC_RegRead(0x0C);
//	//Set up Case statement here to service all of the possible interrupts
//	if ((Int_SourceSystem &0x04)==0x04)
//	{
//		//Perform an Action since Motion Flag has been set
//		//Read the Motion/Freefall Function to clear the interrupt
//		IntSourceMFF=IIC_RegRead(0x16);
//		//Can parse out data to perform a specific action based on the
//		//axes that made the condition true and read the direction of the 
//		//motion event
//	}
//}
