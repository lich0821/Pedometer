#include "simple_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "pca10001.h"
#include "nrf_delay.h"
#include "twi_master.h"
#include "mma8451.h"
#include "ad7151.h"
#include "string.h"
#include "math.h"

//unsigned char reg_value[60];
uint8_t fifoData[60];

void pin_exit(void)
{
	NRF_GPIOTE->CONFIG[0] = ( GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos )
							//( GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos )
							|(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos )
							//|(3 << GPIOTE_CONFIG_PSEL_Pos );	//Cap
							|(24 << GPIOTE_CONFIG_PSEL_Pos );	//MMA
	
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Enabled << GPIOTE_INTENSET_IN0_Pos;
	NVIC_SetPriority(GPIOTE_IRQn, 1);
	NVIC_EnableIRQ(GPIOTE_IRQn);							
}


void GPIOTE_IRQHandler(void)
{
	unsigned char rubish;
	if( (NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk) )
	{
		NRF_GPIOTE->EVENTS_IN[0] = 0; //中断事件清零.
	}
	mma8451_register_read(MMA8451_INT_SOURCE, &rubish, 1);	
	//printf("INT_S:%02X\t", rubish);
	nrf_delay_us(5);
	mma8451_register_read(MMA8451_F_STATUS, &rubish, 1);
	nrf_delay_us(5);
	mma8451_read_fifo(&fifoData[0]);
//	nrf_delay_us(10);
	//printf("INT_S:%02X\r\n", rubish);

}
	
void uart_exit(void)
{
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;

	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);
}

void UART0_IRQHandler(void)
{
	simple_uart_get();
	printf("Interrupt...\r\n");
}

xyzdata_t getMax(xyzdata_t a, xyzdata_t b)
{
	xyzdata_t max;
	max.x = abs(a.x) > abs(b.x) ? a.x : b.x;
	max.y = abs(a.y) > abs(b.y) ? a.y : b.y;
	max.z = abs(a.z) > abs(b.z) ? a.z : b.z;
	
	return max;
}

xyzdata_t getMin(xyzdata_t a, xyzdata_t b)
{
	xyzdata_t min;
	min.x = abs(a.x) < abs(b.x) ? a.x : b.x;
	min.y = abs(a.y) < abs(b.y) ? a.y : b.y;
	min.z = abs(a.z) < abs(b.z) ? a.z : b.z;
	
	return min;
}

xyzdata_t getThreshold(xyzdata_t max, xyzdata_t min)
{
	xyzdata_t res;
	res.x = (max.x + min.x) >> 1;
	res.y = (max.y + min.y) >> 1;
	res.z = (max.z + min.z) >> 1;
	
	return res;
}

xyzdata_t getDelta(xyzdata_t a, xyzdata_t b)
{
	xyzdata_t res;
	res.x = b.x - a.x;
	res.y = b.y - a.y;
	res.z = b.z - a.z;
	
	return res;
}

int main(void)
{
	unsigned char cap_reg[2];
	unsigned short int cap_int;
//	float ax, ay, az;
	xyzdata_t accelValue;
	uint8_t i = 0, j=0;
	xyzdata_t accelData[5];
	xyzdata_t accelDelta[5];
	xyzdata_t accelOld;
	xyzdata_t accelCurrent;
	xyzdata_t accelMax;// = 0;
	xyzdata_t accelMin;// = 32767;
	xyzdata_t threshold;
//	static uint8_t counter=0;
	
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
	
	twi_master_init();
	//nrf_gpio_cfg_input(8, GPIO_PIN_CNF_PULL_Disabled);
	//nrf_gpio_cfg_input(9, GPIO_PIN_CNF_PULL_Disabled);
	uart_exit();
	pin_exit();
	
	
	//ad7151_init(0x90);
	printf("Running...\r\n");

	mma8451_init(0x1C);
	nrf_delay_ms(10);	
	//mma8451_MD_conf();
	mma8451_FIFO_conf();
	
	//accelDelta.x = accelDelta.y = accelDelta.z = 0;
	accelOld = mma8451_deal_fifo(&fifoData[0]);
    while(true)
    {	
		//printf("Cap:%f\n", ad7151_get_cap());
		//memset(&reg_value[0], 0, 0x18);
		//ad7151_register_read(AD7151_DATA_HIGH, &cap_reg[0], 2);
		//cap_int = (cap_reg[0] << 8) + cap_reg[1];	
		//printf("%d\n", cap_int);
		//		nrf_delay_ms(1000);
		//mma8451_get_xyz(&ax, &ay, &az);	
		//printf("%f\t%f\t%f\n", ax, ay, az);	
		//accelValue = mma8451_get_data();
		//printf("%d\t%d\t%d\r\n", accelValue.x, accelValue.y, accelValue.z);
		//printf("%f\t%f\t%f\n", accelValue.x/4096.0, accelValue.y/4096.0, accelValue.z/4096.0);
		//nrf_delay_ms(2000);
//		__WFI();
		//ad7151_init(0x90);
		
		//xyz[i] = mma8451_get_data();
		//mma8451_read_fifo(&accelData[0]);
		//accelData[i++] = mma8451_deal_fifo(&fifoData[0]);
		accelCurrent = mma8451_deal_fifo(&fifoData[0]);
		accelDelta[i++] = getDelta(accelOld, accelCurrent);
		accelOld = accelCurrent;
		if(i==1)
		{
			accelMax.x = accelData[0].x;
			accelMax.y = accelData[0].y;
			accelMax.z = accelData[0].z;
			
			accelMin.x = accelData[0].x;
			accelMin.y = accelData[0].y;
			accelMin.z = accelData[0].z;
		}
		else
		{
			accelMax = getMax(accelMax, accelData[i-1]);
			accelMin = getMin(accelMin, accelData[i-1]);	
		}
		if(i>=5)
		{
			threshold = getThreshold(accelMax, accelMin);
			for(i=0;i<5;i++)
			{
				//printf("%d\t%d\t%d\r\n", accelData[i].x, accelData[i].y, accelData[i].z);
//				accelDelta[i].x = accelData[i].x - accelDelta[i].x;
//				accelDelta[i].y = accelData[i].y - accelDelta[i].x;
//				accelDelta[i].z = accelData[i].z - accelDelta[i].x;
				printf("%d\t%d\t%d\r\n", accelDelta[i].x, accelDelta[i].y, accelDelta[i].z);
			}
//			for(j=1;j<5;j++)
//			{
//				accelDelta.x = accelData[j].x - accelData[j-1].x;
//				accelDelta.y = accelData[j].y - accelData[j-1].y;
//				accelDelta.z = accelData[j].z - accelData[j-1].z;
//				printf("%d\t%d\t%d\r\n", accelDelta.x, accelDelta.y, accelDelta.z);			
//			}			
			//printf("Max: %d\t%d\t%d\r\n", accelMax.x, accelMax.y, accelMax.z);
			//printf("Min: %d\t%d\t%d\r\n", accelMin.x, accelMin.y, accelMin.z);
			//printf("THR: %d\t%d\t%d\r\n", threshold.x, threshold.y, threshold.z);
			i = 0;
		}
		__WFI();
    }
}

/****************************************
+++++++++++++  Low Power  +++++++++++++++
*****************************************

//systemoff
NRF_POWER->TASKS_LOWPWR = 1;
NRF_POWER->SYSTEMOFF = 1;


//watchdog
NRF_WDT->CRV = 327680;
NRF_WDT->TASKS_START = 1;

//wait for interrupt
__WFI();

****************************************/
