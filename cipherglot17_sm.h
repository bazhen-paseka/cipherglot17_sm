#ifndef CIPHERGLOT17_SM_H_INCLUDED
#define CIPHERGLOT17_SM_H_INCLUDED
//**********************************************************************

	#include <stdio.h>
	#include "stdio.h"
	#include <string.h>
	#include <stdlib.h> // rand
	//#include "stm32f1xx_hal.h"
	#include "main.h"

	#include "rtc.h"
	#include "tim.h"
	#include "usart.h"
	#include "gpio.h"

	#include "cipherglot_local_config.h"
	#include "ringbuffer_dma_sm.h"
	#include "flash_stm32f103_hal_sm.h"
	#include "average_calc_3_from_5.h"

//**********************************************************************

	void 	CipherGlot_init	(void);
	void 	CipherGlot_main	(void);

	void 	Prompt_Set		(uint8_t new_prompt_u8);
	void 	Blank_Set		(uint8_t new_blank_u8);
	uint8_t Blank_Status	(void) ;

	void 	Bonus_Start		(uint32_t _tim_period) ;
	void 	Bonus_Stop		(void) ;
	void 	Bonus_Set		(uint8_t _status_u8) ;
	uint8_t	Bonus_Status	(void) ;

	void Blynk_Magic_Set	(uint8_t _key_bonus) ;
	void Blynk_Force_Set_no (uint8_t _key_bonus) ;
	void Blynk_Force_Set_yes(uint8_t _key_bonus) ;


//**********************************************************************

#endif
