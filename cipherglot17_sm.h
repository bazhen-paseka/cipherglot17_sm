#ifndef LCD1602_FC113_SM_H_
#define LCD1602_FC113_SM_H_

	//#include "stm32f1xx_hal.h"

	void CipherGlot_init(void);
	void CipherGlot_main(void);

	void set_Prompt(uint8_t new_prompt_u8);
	void set_Blank(uint8_t new_blank_u8);

#endif
