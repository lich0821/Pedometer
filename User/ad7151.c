#include "ad7151.h"
#include "twi_master.h"
#include "stdio.h"
#include "nrf_delay.h"

static uint8_t m_device_address; 
//extern unsigned char reg_value[0x31];

bool ad7151_init(uint8_t device_address)
{
	bool transfer_succeeded = true;
//	unsigned char i;

	m_device_address = device_address;
	ad7151_register_write(AD7151_RESET, 0x00);	//Reset
	nrf_delay_us(10);
//	for(i=0;i<0x18;i++)
//	{
//		ad7151_register_read(i, &reg_value[i], 1);
//		printf("Reg%02X: %02X\n", i, reg_value[i]);
//	}	
	return transfer_succeeded;
}

bool ad7151_conf(void)
{
	bool transfer_succeeded = true;

	return transfer_succeeded;	
}


bool ad7151_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes)
{
	bool transfer_succeeded = true;
	transfer_succeeded = twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
	transfer_succeeded &= twi_master_transfer(m_device_address|TWI_READ_BIT, destination, number_of_bytes, TWI_ISSUE_STOP);	
	return transfer_succeeded;
}

bool ad7151_register_write(uint8_t register_address, uint8_t value)
{
	uint8_t w2_data[2];

	w2_data[0] = register_address;
	w2_data[1] = value;
	return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);	
}

float ad7151_get_cap( void )
{
	float cap_value;
	unsigned char cap_reg[2];
	unsigned short int cap_int;
	
	ad7151_register_read(AD7151_DATA_HIGH, &cap_reg[0], 2);
	cap_int = (cap_reg[0] << 8) + cap_reg[1];
	
	if(cap_int < 0x3000 )	//Data rage error
	{
		return -1.0;
	}
	else if( cap_int > 0xCFF0 )
		return 10.0;
	
	cap_value = (cap_int - 12288) / 10236.0;//(1/40944.0) * 4;
	
	return cap_value;
}

