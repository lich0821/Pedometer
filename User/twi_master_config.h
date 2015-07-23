/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef TWI_MASTER_CONFIG
#define TWI_MASTER_CONFIG

//#define TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER (1U)
//#define TWI_MASTER_CONFIG_DATA_PIN_NUMBER (0U)

////PCB1 was wrong and set CLK to 9, SDA to 8 when using MMA8451
//#define TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER (9U)
//#define TWI_MASTER_CONFIG_DATA_PIN_NUMBER (8U)

//PCB1 was wrong and set CLK to 8, SDA to 9 when using AD7151
#define TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER (9U)
#define TWI_MASTER_CONFIG_DATA_PIN_NUMBER (8U)

#endif
