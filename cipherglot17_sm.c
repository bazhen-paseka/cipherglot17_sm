#define SOFT_VERSION 	362

	// TIM2 Sound on port PA1
	// TIM3 timer LED Off = 2 sec
	// TIM4 prompt current cipher
	// UART3 -> PB10 for DEBUG, speed = 38400;

	// Segment_A PA  1
	// Segment_B PC 13
	// Segment_C PC 15
	// Segment_D PA  2
	// Segment_E PA  3
	// Segment_F PA  6
	// Segment_G PA  7
	// Segment_P PC 14

#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

	#include <string.h>
	#include <stdlib.h> // rand
	#include "cipherglot17_sm.h"
	#include "ringbuffer_dma_sm.h"
	#include "flash_stm32f103_hal_sm.h"

//**********************************************************************

RTC_TimeTypeDef TimeStruct  ;

#define END_NUMBER 1024

uint8_t cipher_arr_u8[END_NUMBER] = {3,
	1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5,0,2,8,8,4,1,9,7,1,6,9,3,9,9,3,7,5,1,0,
	5,8,2,0,9,7,4,9,4,4,5,9,2,3,0,7,8,1,6,4,0,6,2,8,6,2,0,8,9,9,8,6,2,8,0,3,4,8,2,5,3,4,2,1,1,7,0,6,7,9,
	8,2,1,4,8,0,8,6,5,1,3,2,8,2,3,0,6,6,4,7,0,9,3,8,4,4,6,0,9,5,5,0,5,8,2,2,3,1,7,2,5,3,5,9,4,0,8,1,2,8,
	4,8,1,1,1,7,4,5,0,2,8,4,1,0,2,7,0,1,9,3,8,5,2,1,1,0,5,5,5,9,6,4,4,6,2,2,9,4,8,9,5,4,9,3,0,3,8,1,9,6
	//		  44288 10975 66593 34461 28475  64823 37867 83165 27120 19091
	//		  45648 56692 34603 48610 45432  66482 13393 60726 02491 41273
	//		  72458 70066 06315 58817 48815  20920 96282 92540 91715 36436
	//		  78925 90360 01133 05305 48820  46652 13841 46951 94151 16094
	//		  33057 27036 57595 91953 09218  61173 81932 61179 31051 18548
	//		  07446 23799 62749 56735 18857  52724 89122 79381 83011 94912
	//		  98336 73362 44065 66430 86021  39494 63952 24737 19070 21798
	//		  60943 70277 05392 17176 29317  67523 84674 81846 76694 05132
	//		  00056 81271 45263 56082 77857  71342 75778 96091 73637 17872
	//		  14684 40901 22495 34301 46549  58537 10507 92279 68925 89235
	//		  42019 95611 21290 21960 86403  44181 59813 62977 47713 09960
	//		  51870 72113 49999 99837 29780  49951 05973 17328 16096 31859
	//		  50244 59455 34690 83026 42522  30825 33446 85035 26193 11881
	//		  71010 00313 78387 52886 58753  32083 81420 61717 76691 47303
	//		  59825 34904 28755 46873 11595  62863 88235 37875 93751 95778
	//		  18577 80532 17122 68066 13001  92787 66111 95909 21642 01989
};

uint8_t blank_u8 		= 0 ;
uint8_t prompt_u8 		= 0 ;
uint8_t error_status_u8	= 0 ;
uint8_t game_type_u8	= 3 ; // Pi or Old

uint32_t start_cipher_number_u32   = 0 ;
uint32_t current_cipher_number_u32 = 0 ;
uint32_t total_cipher_number_u32   = 0 ;
uint32_t previous_cipher_number_u32= 0 ;

char DataChar[100];
//**********************************************************************

void Segment_A(uint8_t);	// A	бело-зеленый
void Segment_B(uint8_t);	// B	зеленый
void Segment_C(uint8_t);	// C	бело-оранжевый
void Segment_D(uint8_t);	// D	cиний
void Segment_E(uint8_t);	// E	бело-cиний
void Segment_F(uint8_t);	// F	оранжевый
void Segment_G(uint8_t);	// G	бело-коричневый
void Segment_P(uint8_t);	// p	коричневый

void ScanRow_123A(uint8_t);
void ScanRow_456B(uint8_t);
void ScanRow_789C(uint8_t);
void ScanRow_E0FD(uint8_t);

void Generate_New_Cipher(void);
void CipherPrint (uint8_t);
void BeepCipher_OK(uint8_t _cipher);
void BeepCipherError(uint32_t, uint32_t, uint32_t, uint8_t, uint8_t, uint8_t);
void Beeper (uint8_t);
void Beeper_11(void);
void Beeper_12(void);
void BeepError2 (void);
void BeepError3 (void);
void TheEnd (void);
void Test_Segment (void);

void BlankIndicatorStart (void);
void BlankIndicatorStop (void);

uint8_t ScanKeyBoard (void);
uint8_t KeyPressed (void);
uint8_t TestLED (void);
uint8_t Prompt_Status (void);

void PrintSoftVersion(int *_soft_version_arr_int);
void Prompt_Start(void);

//**********************************************************************

void CipherGlot_init(void) {
	MX_RTC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_USART1_UART_Init();

	//HAL_TIM_Base_Start_IT(&htim3); // start TIM3 interupt
	BlankIndicatorStart();

	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100)     ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	sprintf(DataChar,"\r\n CipherGlot-17 2020-jan-19 v%d.%d.%d Migrate CubeMX\r\nUART1 for debug started on speed 38400\r\n",
			soft_version_arr_int[0],soft_version_arr_int[2],soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	sprintf(DataChar,"Press:\r\n 1 - load previous;\r\n 3 - load Pi;\r\n any key - start new.\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	Test_Segment();
	PrintSoftVersion(soft_version_arr_int);
	game_type_u8 = TestLED();

	switch (game_type_u8) {
		case 1: {	// read Flash
			sprintf(DataChar,"\r\n-> Start read from Flash:\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			uint32_t flash_value_u32;

			flash_value_u32 = Flash_Read(MY_FLASH_PAGE_ADDR);
			start_cipher_number_u32 = (flash_value_u32 & 0x0000ffff)       ;
			total_cipher_number_u32 = ((flash_value_u32 & 0xffff0000)>>16) ;
			previous_cipher_number_u32 = total_cipher_number_u32;

			flash_value_u32 = Flash_Read(MY_FLASH_PAGE_ADDR + 4);	//	read Time
			TimeStruct.Hours   = (flash_value_u32 & 0x00ff0000) >> 16 ;
			TimeStruct.Minutes = (flash_value_u32 & 0x0000ff00) >> 8  ;
			TimeStruct.Seconds = (flash_value_u32 & 0x000000ff)       ;
			HAL_RTC_SetTime( &hrtc, &TimeStruct, RTC_FORMAT_BIN );

			sprintf(DataChar,"load time %u:%02u:%02u\r\n", TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);


			sprintf(DataChar,"start_cipher_number: %d\r\ntotal_cipher_number: %d\r\n", (int)start_cipher_number_u32, (int)(total_cipher_number_u32 - start_cipher_number_u32));
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			for (uint32_t flash_addr_u32 = 0; flash_addr_u32 < 0xFA; flash_addr_u32++) {
				flash_value_u32 = Flash_Read(MY_FLASH_PAGE_ADDR + (flash_addr_u32 + 2) * 4) ;

				cipher_arr_u8[flash_addr_u32 * 4 + 1 ] = (flash_value_u32 & 0x000000ff)     ;
				cipher_arr_u8[flash_addr_u32 * 4 + 2 ] = (flash_value_u32 & 0x0000ff00)>>8  ;
				cipher_arr_u8[flash_addr_u32 * 4 + 3 ] = (flash_value_u32 & 0x00ff0000)>>16 ;
				cipher_arr_u8[flash_addr_u32 * 4 + 4 ] = (flash_value_u32 & 0xff000000)>>24 ;

				if ((flash_addr_u32 != 0) && (flash_addr_u32*4 < total_cipher_number_u32)) {
					for (uint8_t i = 1; i < 5; i++) {
						if ((flash_addr_u32 * 4 + i) <= total_cipher_number_u32) {
							sprintf(DataChar,"%X", cipher_arr_u8[flash_addr_u32 * 4 + i] );
							HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

							if (i == 4) {
								sprintf(DataChar,"; \t\t");
								HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
							}
						}
					}

					if (flash_addr_u32 % 5 == 0) {
						sprintf(DataChar,"\r\n");
						HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
					}
				}
			}
			sprintf(DataChar,"\r\n-> Finish read from flash.\r\n\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			current_cipher_number_u32 = start_cipher_number_u32;
		}
		break;

		case 3: {
			start_cipher_number_u32 = 0;	// use 'Pi'
			total_cipher_number_u32   = start_cipher_number_u32 ;
			current_cipher_number_u32 = start_cipher_number_u32 ;
		}
		break;

		default: {
		  start_cipher_number_u32 = 5 ;
		  do {
			  cipher_arr_u8[1] = rand()%10; // 0..9
			  cipher_arr_u8[2] = rand()%10;
			  cipher_arr_u8[3] = rand()%10;
			  cipher_arr_u8[4] = rand()%10;
		  }
		  while (	(cipher_arr_u8[1] == cipher_arr_u8[2])
				 ||	(cipher_arr_u8[2] == cipher_arr_u8[3])
				 ||	(cipher_arr_u8[3] == cipher_arr_u8[1])  );

		total_cipher_number_u32   = start_cipher_number_u32 ;
		current_cipher_number_u32 = start_cipher_number_u32 ;
		} // default
		break;
	}

	sprintf(DataChar,"Init - Ok\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	//	HAL_TIM_Base_Start_IT(&htim4); // start TIM4 prompt
	Prompt_Start();
}
//**********************************************************************

void CipherGlot_main(void) {
	if (total_cipher_number_u32 == start_cipher_number_u32) {
		TimeStruct.Hours   = 0;
		TimeStruct.Minutes = 0;
		TimeStruct.Seconds = 0;
		HAL_RTC_SetTime( &hrtc, &TimeStruct, RTC_FORMAT_BIN );
	}

	Generate_New_Cipher();

	if ((game_type_u8 == 3) && (total_cipher_number_u32 == start_cipher_number_u32) ) {
		uint8_t start_numb_arr_u8[3]  = {0,0,0} ;

		Beeper_11();
		CipherPrint(0x11);
		Segment_A(1);
		start_numb_arr_u8[0] = KeyPressed();
		Beeper_12();
		CipherPrint(start_numb_arr_u8[0]);
		HAL_Delay(500);

		CipherPrint(0x11);
		Segment_G(1);
		start_numb_arr_u8[1] = KeyPressed();
		Beeper_12();
		CipherPrint(start_numb_arr_u8[1]);
		HAL_Delay(500);

		CipherPrint(0x11);
		Segment_D(1);
		start_numb_arr_u8[2] = KeyPressed();
		Beeper_12();
		CipherPrint(start_numb_arr_u8[2]);
		HAL_Delay(300);
		total_cipher_number_u32 = 100*start_numb_arr_u8[0] + 10*start_numb_arr_u8[1] + start_numb_arr_u8[2];
	}

	Beeper(cipher_arr_u8[total_cipher_number_u32]);
	CipherPrint(cipher_arr_u8[total_cipher_number_u32]);
	current_cipher_number_u32 = start_cipher_number_u32;

	TIM4->CNT = 0;
	HAL_TIM_Base_Start(&htim4);
	do {  // Compare
		sprintf(DataChar," %X", (int)cipher_arr_u8[current_cipher_number_u32]);	// hint current Cipher
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		if ( KeyPressed() == cipher_arr_u8[current_cipher_number_u32]) {
			error_status_u8 = 0;
			CipherPrint(cipher_arr_u8[current_cipher_number_u32]);
			BeepCipher_OK(cipher_arr_u8[current_cipher_number_u32]);
			current_cipher_number_u32++;

			TIM4->CNT = 0;
			set_Prompt(0);
		}
		else {
			error_status_u8++;

			if (error_status_u8 == 1) {
				sprintf(DataChar," Error #1\r\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

				BeepError2();
				CipherPrint(cipher_arr_u8[current_cipher_number_u32]);
				current_cipher_number_u32 = start_cipher_number_u32;

			}
			else {
				HAL_TIM_Base_Stop(&htim4); // stop TIM4 prompt

				sprintf(DataChar," Error #2\r\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

				BeepError3();

				HAL_RTC_GetTime( &hrtc, &TimeStruct, RTC_FORMAT_BIN );
				uint8_t stop_hours_u8   = TimeStruct.Hours   ;
				uint8_t stop_minutes_u8 = TimeStruct.Minutes ;
				uint8_t stop_second_u8  = TimeStruct.Seconds  ;

				BeepCipherError(start_cipher_number_u32, current_cipher_number_u32, total_cipher_number_u32 + 1, stop_hours_u8, stop_minutes_u8, stop_second_u8 );

				CipherPrint(0x0F);
				BlankIndicatorStop();
				sprintf(DataChar,"** Press button 'F' to write to flash **\r\n");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

				if (KeyPressed() == 0x0F) {
					Beeper_11();
					CipherPrint(0x11);
					HAL_Delay(300);
					CipherPrint(0x0F);
					// write to Flash:
					sprintf(DataChar,"\r\n-> Start write to flash:\r\n");
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

					HAL_FLASH_Unlock();
					Flash_Erase_Page(MY_FLASH_PAGE_ADDR);

					uint32_t flash_value_u32;

					sprintf(DataChar,"start_cipher_number: %d\r\ntotal_cipher_number: %d\r\n", (int)start_cipher_number_u32, (int)total_cipher_number_u32);
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

					flash_value_u32 = start_cipher_number_u32 + (total_cipher_number_u32 << 16);
					Flash_Write( MY_FLASH_PAGE_ADDR, flash_value_u32);

					flash_value_u32 = (stop_hours_u8 << 16) + (stop_minutes_u8 << 8) + stop_second_u8;
					Flash_Write( MY_FLASH_PAGE_ADDR + 4, flash_value_u32);

					sprintf(DataChar,"write time %u:%02u:%02u\r\n", stop_hours_u8, stop_minutes_u8, stop_second_u8);
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

					for (uint32_t flash_addr_u32 = 0; flash_addr_u32 < 0xFA; flash_addr_u32++) {
						flash_value_u32 = (cipher_arr_u8[flash_addr_u32 * 4 + 1 ]       )
										+ (cipher_arr_u8[flash_addr_u32 * 4 + 2 ] <<  8 )
										+ (cipher_arr_u8[flash_addr_u32 * 4 + 3 ] << 16 )
										+ (cipher_arr_u8[flash_addr_u32 * 4 + 4 ] << 24 ) ;
						Flash_Write( (MY_FLASH_PAGE_ADDR + (flash_addr_u32 + 2) * 4), flash_value_u32);
					}
					HAL_FLASH_Lock();
					sprintf(DataChar,"-> Finish write to flash.\r\n\r\n");
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				}
				else {
					Beeper_11();
					CipherPrint(0x15);
					sprintf(DataChar,"-- NO write to Flash --\r\n");
					HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				}

				HAL_Delay(2000);
				BlankIndicatorStart();

				if (game_type_u8 != 1) {
					total_cipher_number_u32 = start_cipher_number_u32;
				}
			}
		}
	} // do Komp
	while ((current_cipher_number_u32 <= total_cipher_number_u32 ) && ( error_status_u8 < 2 ));

	HAL_TIM_Base_Stop(&htim4); // stop TIM4 prompt
	if ( error_status_u8 == 0 ) {
		total_cipher_number_u32++;
		sprintf(DataChar," qnt: %d\r\n", (int)(1 + total_cipher_number_u32 - start_cipher_number_u32));
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}

	if (total_cipher_number_u32 >= END_NUMBER) {
		TheEnd();
	}
}
//**********************************************************************

void BeepCipherError(uint32_t StartNumb, uint32_t CurNumb, uint32_t MaxNumb, uint8_t _StopHour_u8, uint8_t _StopMin_u8, uint8_t _StopSec_u8) {
	// ERROR

	//  COUT Current Number
		//	KeyPressed();
		//	Beeper_12();					HAL_Delay(300);
		//
		//	Beeper_11();
		//	CipherPrint(CurNumb/100);	HAL_Delay(500);
		//	CipherPrint(0x11);			HAL_Delay(100);
		//
		//	Beeper_11();
		//	CurNumb = CurNumb%100;
		//	CipherPrint(CurNumb/10);	HAL_Delay(500);
		//	CipherPrint(0x11);			HAL_Delay(100);
		//
		//	Beeper_11();
		//	CurNumb = CurNumb%10;
		//	CipherPrint(CurNumb);		HAL_Delay(500);
		//	CipherPrint(0x11);			HAL_Delay(500);


	//  COUT Total Number
	CipherPrint(0x25); // 'n'
	sprintf(DataChar,"Press any button to see total_cipher_numb\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	KeyPressed();
	//Beeper_12();					HAL_Delay(300);
	Beeper_12();						HAL_Delay(300);

	MaxNumb = MaxNumb - StartNumb;

	sprintf(DataChar,"Total cipher numb: %d\r\n", (int)MaxNumb);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	Beeper_11();
	CipherPrint(MaxNumb/100);		HAL_Delay(500);
	CipherPrint(0x11);				HAL_Delay(100);

	Beeper_11();
	CipherPrint((MaxNumb%100)/10);	HAL_Delay(500);
	CipherPrint(0x11);				HAL_Delay(100);

	Beeper_11();
	CipherPrint(MaxNumb%10);		HAL_Delay(500);

	CipherPrint(0x26);				HAL_Delay(500);	// 't'

	// COUT TIME
	sprintf(DataChar,"Press any button to see time\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	KeyPressed();

	sprintf(DataChar,"Time %d:%02d:%02d\r\n", (int)_StopHour_u8, (int)_StopMin_u8, (int)_StopSec_u8);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	//Beeper_12();					HAL_Delay(300);
	Beeper_12();						HAL_Delay(300);
	Beeper_12();						HAL_Delay(300);

	Beeper_11();
	CipherPrint(_StopHour_u8 % 10);	HAL_Delay(500);
	CipherPrint(0x11);				HAL_Delay(100);

	Beeper_11();
	CipherPrint(_StopMin_u8 / 10);	HAL_Delay(500);
	CipherPrint(0x11);				HAL_Delay(100);

	Beeper_11();
	CipherPrint(_StopMin_u8 % 10);	HAL_Delay(500);

	CipherPrint(0x0E);
	BlankIndicatorStop();
	sprintf(DataChar,"Press button 'E' to send on ThingSpeak\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	if (KeyPressed() == 0x0E) {
		CipherPrint(0x0E);
		Beeper_11();
		char http_req[200];
		sprintf(http_req, "&field8=%d\r\n\r\n", (int)MaxNumb );
		RingBuffer_DMA_Connect();
		CipherPrint(0x11);
		Beeper_11();
		CipherPrint(0x0E);

		RingBuffer_DMA_Main(http_req);
		Beeper_11();
		CipherPrint(0x11);
		//HAL_Delay(1000);
	}
	else {
		Beeper_11();
		CipherPrint(0x24);
		sprintf(DataChar,"-- NO send to ThingSpeak --\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		HAL_Delay(1000);
	}
	BlankIndicatorStart();
//	KeyPressed();
//	Beeper_11();
//	HAL_Delay(1000);
}
//**********************************************************************

void Generate_New_Cipher (void) {
	if (game_type_u8 == 3)	return;
	if ((game_type_u8 == 1) && (current_cipher_number_u32 <= previous_cipher_number_u32)) return;

	//		Generate_New_Cipher with letter char
	//	if ( total_cipher_number_u32 % 4 == 0 ) {
	//		do {
	//			cipher_arr_u8[total_cipher_number_u32] = rand()%6 + 10; // generate char: 'A' .. 'F'
	//		}
	//		while ( (cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 4]) );
	//	}
	//	else {
	//		do {
	//			cipher_arr_u8[total_cipher_number_u32] = rand()%10; // 0x00 .. 0x0F
	//		}
	//		while ( (cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 1]) ||
	//				(cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 2]) ||
	//				(cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 3]) ||
	//				(cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 4])  );
	//	}

	do {
		cipher_arr_u8[total_cipher_number_u32] = rand()%10; // 0x00 .. 0x0F
	}
	while ( (cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 1]) ||
			(cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 2]) ||
			(cipher_arr_u8[total_cipher_number_u32] == cipher_arr_u8[total_cipher_number_u32 - 3]) );

	sprintf(DataChar,"new Cipher: %X\r\n", cipher_arr_u8[total_cipher_number_u32]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
}
//--------------------------------------------------------------------------


uint8_t ScanKeyBoard(void) {
	uint8_t keyboard_u8;

	TIM4->CNT = 0;
	set_Prompt(0);

	static uint8_t previous_cipher_u8;
	do {
		if (Prompt_Status() == 1 ) {
			if (previous_cipher_u8 != cipher_arr_u8[current_cipher_number_u32]) {
				sprintf(DataChar," hint: %X\r\n", (int)cipher_arr_u8[current_cipher_number_u32]);
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
				previous_cipher_u8 = cipher_arr_u8[current_cipher_number_u32];
			}

			CipherPrint(cipher_arr_u8[current_cipher_number_u32]);
			HAL_Delay(200);
			CipherPrint(0x11);
			set_Prompt(0);
			TIM4->CNT = 0;
		}

		if (blank_u8 == 1) {
			CipherPrint(0x11) ; // blank
			blank_u8 = 0;
		}
		rand();
		keyboard_u8 = 0xFF;

		ScanRow_123A(1);
		if (KEYBOARD_COLUMN_1_GPIO_Port->IDR & KEYBOARD_COLUMN_1_IDR) keyboard_u8 =0x01;
		if (KEYBOARD_COLUMN_2_GPIO_Port->IDR & KEYBOARD_COLUMN_2_IDR) keyboard_u8 =0x02;
		if (KEYBOARD_COLUMN_3_GPIO_Port->IDR & KEYBOARD_COLUMN_3_IDR) keyboard_u8 =0x03;
		if (KEYBOARD_COLUMN_A_GPIO_Port->IDR & KEYBOARD_COLUMN_A_IDR) keyboard_u8 =0x0A;
		ScanRow_123A(0);

		ScanRow_456B(1);
		if (KEYBOARD_COLUMN_1_GPIO_Port->IDR & KEYBOARD_COLUMN_1_IDR) keyboard_u8 =0x04;
		if (KEYBOARD_COLUMN_2_GPIO_Port->IDR & KEYBOARD_COLUMN_2_IDR) keyboard_u8 =0x05;
		if (KEYBOARD_COLUMN_3_GPIO_Port->IDR & KEYBOARD_COLUMN_3_IDR) keyboard_u8 =0x06;
		if (KEYBOARD_COLUMN_A_GPIO_Port->IDR & KEYBOARD_COLUMN_A_IDR) keyboard_u8 =0x0B;
		ScanRow_456B(0);

		ScanRow_789C(1);
		if (KEYBOARD_COLUMN_1_GPIO_Port->IDR & KEYBOARD_COLUMN_1_IDR) keyboard_u8 =0x07;
		if (KEYBOARD_COLUMN_2_GPIO_Port->IDR & KEYBOARD_COLUMN_2_IDR) keyboard_u8 =0x08;
		if (KEYBOARD_COLUMN_3_GPIO_Port->IDR & KEYBOARD_COLUMN_3_IDR) keyboard_u8 =0x09;
		if (KEYBOARD_COLUMN_A_GPIO_Port->IDR & KEYBOARD_COLUMN_A_IDR) keyboard_u8 =0x0C;
		ScanRow_789C(0);

		ScanRow_E0FD(1);
		if (KEYBOARD_COLUMN_1_GPIO_Port->IDR & KEYBOARD_COLUMN_1_IDR) keyboard_u8 =0x0E;
		if (KEYBOARD_COLUMN_2_GPIO_Port->IDR & KEYBOARD_COLUMN_2_IDR) keyboard_u8 =0x00;
		if (KEYBOARD_COLUMN_3_GPIO_Port->IDR & KEYBOARD_COLUMN_3_IDR) keyboard_u8 =0x0F;
		if (KEYBOARD_COLUMN_A_GPIO_Port->IDR & KEYBOARD_COLUMN_A_IDR) keyboard_u8 =0x0D;
		ScanRow_E0FD(0);
		}
	while (keyboard_u8 == 0xFF);
	return keyboard_u8;
}
//**********************************************************************

void Segment_A(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_A_GPIO_Port, LED_SEGMENT_A_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_A_GPIO_Port, LED_SEGMENT_A_Pin, RESET);
}
//****************************

void Segment_B(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_B_GPIO_Port, LED_SEGMENT_B_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_B_GPIO_Port, LED_SEGMENT_B_Pin, RESET);
}
//****************************

void Segment_C(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_C_GPIO_Port, LED_SEGMENT_C_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_C_GPIO_Port, LED_SEGMENT_C_Pin, RESET);
}
//****************************

void Segment_D(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_D_GPIO_Port, LED_SEGMENT_D_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_D_GPIO_Port, LED_SEGMENT_D_Pin, RESET);
}
//****************************

void Segment_E(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_E_GPIO_Port, LED_SEGMENT_E_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_E_GPIO_Port, LED_SEGMENT_E_Pin, RESET);
}
//****************************

void Segment_F(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_F_GPIO_Port, LED_SEGMENT_F_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_F_GPIO_Port, LED_SEGMENT_F_Pin, RESET);
}
//****************************

void Segment_G(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_G_GPIO_Port, LED_SEGMENT_G_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_G_GPIO_Port, LED_SEGMENT_G_Pin, RESET);
}
//****************************

void Segment_P(uint8_t _status) {
	if (_status == 0) HAL_GPIO_WritePin(LED_SEGMENT_P_GPIO_Port, LED_SEGMENT_P_Pin,   SET);
	else              HAL_GPIO_WritePin(LED_SEGMENT_P_GPIO_Port, LED_SEGMENT_P_Pin, RESET);
}
//**********************************************************************


void ScanRow_123A(uint8_t status) {
	if (status == 0) HAL_GPIO_WritePin(KEYBOARD_ROW_123A_GPIO_Port, KEYBOARD_ROW_123A_Pin,  RESET);
	else			 HAL_GPIO_WritePin(KEYBOARD_ROW_123A_GPIO_Port, KEYBOARD_ROW_123A_Pin,    SET);
}
//****************************

void ScanRow_456B(uint8_t status) {
	if (status == 0) HAL_GPIO_WritePin(KEYBOARD_ROW_456B_GPIO_Port, KEYBOARD_ROW_456B_Pin,  RESET);
	else			 HAL_GPIO_WritePin(KEYBOARD_ROW_456B_GPIO_Port, KEYBOARD_ROW_456B_Pin,    SET);
}
//****************************

void ScanRow_789C(uint8_t status) {
	if (status == 0) HAL_GPIO_WritePin(KEYBOARD_ROW_789C_GPIO_Port, KEYBOARD_ROW_789C_Pin,  RESET);
	else			 HAL_GPIO_WritePin(KEYBOARD_ROW_789C_GPIO_Port, KEYBOARD_ROW_789C_Pin,    SET);
}
//****************************

void ScanRow_E0FD(uint8_t status) {
	if (status == 0) HAL_GPIO_WritePin(KEYBOARD_ROW_E0FD_GPIO_Port, KEYBOARD_ROW_E0FD_Pin,  RESET);
	else			 HAL_GPIO_WritePin(KEYBOARD_ROW_E0FD_GPIO_Port, KEYBOARD_ROW_E0FD_Pin,    SET);
}
//**********************************************************************

void Test_Segment(void) {
	Segment_A(0);
	Segment_B(0);
	Segment_C(0);
	Segment_P(0);
	Segment_D(0);
	Segment_G(0);
	Segment_F(0);
	Segment_E(0);

	#define SEGMENT_SHOW_MS	200

	Beeper_11();
	Segment_B(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_B(0);

	Beeper_11();
	Segment_C(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_C(0);

	Beeper_11();
	Segment_E(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_E(0);

	Beeper_11();
	Segment_F(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_F(0);

	Beeper_11();
	Segment_A(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_A(0);

	Beeper_11();
	Segment_G(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_G(0);

	Beeper_11();
	Segment_D(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_D(0);

	Beeper_11();
	Segment_P(1);
	HAL_Delay(SEGMENT_SHOW_MS);
	Segment_P(0);
}
//**********************************************************************

void CipherPrint (uint8_t num) {
//	-A-A-
//	F---B
//	-G-G-
//	E---C
//	-D-D-
//	----- P
	TIM3->CNT = 0; //
	if (num==0x00) {	 // "0"
			Segment_A(1);
		Segment_F(1);Segment_B(1);
			Segment_G(0);
		Segment_E(1);Segment_C(1);
			Segment_D(1);
					Segment_P(0);
		}

	if (num==0x01) {	// "1"
		Segment_A(0);
	Segment_F(0);Segment_B(1);
		Segment_G(0);
	Segment_E(0);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x02)		// "2"
		{
		Segment_A(1);
	Segment_F(0);Segment_B(1);
		Segment_G(1);
	Segment_E(1);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x03)		// "3"
		{
		Segment_A(1);
	Segment_F(0);Segment_B(1);
		Segment_G(1);
	Segment_E(0);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x04)		// "4"
		{
		Segment_A(0);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(0);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x05)		// "5"
		{
		Segment_A(1);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(0);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x06) {		// "6"
		Segment_A(1);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x07) {		// "7"
		Segment_A(1);
	Segment_F(0);Segment_B(1);
		Segment_G(0);
	Segment_E(0);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x08) {		// "8"
		Segment_A(1);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x09) {		// "9"
		Segment_A(1);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(0);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x0A) {		// "A"
		Segment_A(1);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x0B) {		// "b"
		Segment_A(0);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x0C) {		// "C"
		Segment_A(1);
	Segment_F(1);Segment_B(0);
		Segment_G(0);
	Segment_E(1);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x0D) {		// "d"
		Segment_A(0);
	Segment_F(0);Segment_B(1);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x0E) {		// "E"
		Segment_A(1);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x0F) {		// "F"
		Segment_A(1);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(0);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x10) {		// "point"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(0);Segment_C(0);
		Segment_D(0);
				Segment_P(1);
		}

	if (num==0x11) {		// "blank"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(0);Segment_C(0);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x12) {		// "error-1"
		Segment_A(0);
	Segment_F(1);Segment_B(1);
		Segment_G(0);
	Segment_E(1);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x13) {		// "error-2"
		Segment_A(1);
	Segment_F(0);Segment_B(0);
		Segment_G(1);
	Segment_E(0);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x14) {		// "L"
		Segment_A(0);
	Segment_F(1);Segment_B(0);
		Segment_G(0);
	Segment_E(1);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x15) {		// "o" down
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x16)		// "u"
		{
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x17) {		// "H"
		Segment_A(0);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x18) {		// "i"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(0);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}


	if (num==0x21) {		// "-1"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(0);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x22) {		// "-2"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(1);
	Segment_E(0);Segment_C(0);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x23) {		// "-3"
		Segment_A(1);
	Segment_F(0);Segment_B(0);
		Segment_G(0);
	Segment_E(0);Segment_C(0);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x24) {		// "o" upper
		Segment_A(1);
	Segment_F(1);Segment_B(1);
		Segment_G(1);
	Segment_E(0);Segment_C(0);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x25) {		// "n"
		Segment_A(0);
	Segment_F(0);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(1);
		Segment_D(0);
				Segment_P(0);
		}

	if (num==0x26) {		// "t"
		Segment_A(0);
	Segment_F(1);Segment_B(0);
		Segment_G(1);
	Segment_E(1);Segment_C(0);
		Segment_D(1);
				Segment_P(0);
		}

	if (num==0x27) {		// "U"
		Segment_A(0);
	Segment_F(1);Segment_B(1);
		Segment_G(0);
	Segment_E(1);Segment_C(1);
		Segment_D(1);
				Segment_P(0);
		}
}
//**********************************************************************

uint8_t TestLED(void) {
	uint8_t start_key_u8 = 0;

	Beeper_12();
	Beeper_11();
	start_key_u8 = KeyPressed();
	Beeper_11();
	srand(start_key_u8);
	CipherPrint(start_key_u8);
	HAL_Delay(500);

	if (start_key_u8 != 1) {
		for (uint32_t i=0; i<=start_key_u8 ; i++) {
			Beeper_11();
			CipherPrint(i);
			HAL_Delay(300);
		}
	}

	CipherPrint(0x11); // blank
	rand(); // проcто так
	HAL_Delay(1000);
	return start_key_u8;
}
//**********************************************************************

void PrintSoftVersion(int *_soft_version_arr_int) {
	HAL_Delay(1000);
	Beeper_12();
	Beeper_11();
	HAL_Delay(500);
	for (uint32_t i=0; i<3 ; i++) {
		Beeper_11();
		CipherPrint(_soft_version_arr_int[i]);
		HAL_Delay(300);
	}
	CipherPrint(0x11); // blank
	HAL_Delay(1000);
}
//**********************************************************************

uint8_t KeyPressed(void) {
	uint8_t key1_u8;
	uint8_t key2_u8;

	do { // KeyPresseed
		blank_u8 = 0;
		key1_u8 = ScanKeyBoard();
		HAL_Delay(20);
		blank_u8 = 0;
		key2_u8 = ScanKeyBoard();
	}
	while (key1_u8 != key2_u8);

	return key1_u8;
}
//**********************************************************************

void Beeper_12(void) {
	TIM2->CNT = 0;
	TIM2->ARR  = 2000;
	TIM2->CCR1 = 1000;
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_Delay(80);
	HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
}
//**********************************************************************

void Beeper_11(void) {
	TIM2->CNT = 0;
	TIM2->ARR  = 1000;
	TIM2->CCR1 =  500;
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_Delay(20);
	HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
}
//**********************************************************************

void Beeper(uint8_t type) {
	uint32_t nota_arr_u32[12];	//	arr = 2`000`000 / F;
//	nota_arr_u32[ 0] = 7692;		//	260	do C4
//	nota_arr_u32[ 1] = 6800;		//	294 re D4
//	nota_arr_u32[ 2] = 6060;		//	330 me E4
//	nota_arr_u32[ 3] = 5714;		//	350 fa F4
//	nota_arr_u32[ 4] = 5128;		//	390 so G4
//	nota_arr_u32[ 5] = 4544;		//	440 la A4
//	nota_arr_u32[ 6] = 4048;		//	494 se B4
//	nota_arr_u32[ 7] = 3816;		//	524 do C5
//	nota_arr_u32[ 8] = 3400;		//	588 re D5
//	nota_arr_u32[ 9] = 3030;		//	660 me E5
//	nota_arr_u32[10] = 2856;		//	700 fa F5
//	nota_arr_u32[11] = 2564;		//	780 so G5

	nota_arr_u32[ 0] = 6804;		//	re	D4	294
	nota_arr_u32[ 1] = 5732;		//	fa	F4	349
	nota_arr_u32[ 2] = 4546;		//	la	A4	440
	nota_arr_u32[ 3] = 3824;		//	do	C5	523
	nota_arr_u32[ 4] = 3036;		//	me	E5	659
	nota_arr_u32[ 5] = 2554;		//	so	G5	783
	nota_arr_u32[ 6] = 2024;		//	se	B5	988
	nota_arr_u32[ 7] = 1704;		//	re	D6	1174
	nota_arr_u32[ 8] = 1433;		//	fa	F6	1396
	nota_arr_u32[ 9] = 1136;		//	la	A6	1760
	nota_arr_u32[10] =  956;		//	do	C7	2093
	nota_arr_u32[11] =  758;		//	me	E7	2637

	TIM2->CNT = 0;
	TIM2->ARR  = (nota_arr_u32[type])      ;
	TIM2->CCR1 = (nota_arr_u32[type]) >> 1 ;
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_Delay(50);
	HAL_TIM_PWM_Stop (&htim2,TIM_CHANNEL_1);
}
//**********************************************************************

void TheEnd(void) {
	Beeper_12();
	Beeper_11();
	Beeper_12();
	CipherPrint(0x09);
}
//**********************************************************************

void BeepCipher_OK(uint8_t _cipher) {
	Beeper(_cipher);
	//CipherPrint(0x10); // point
	HAL_Delay(200);
	//CipherPrint(0x11); // blank
}
//**********************************************************************

uint8_t Prompt_Status(void) {
	return prompt_u8;
}
//**********************************************************************

void set_Prompt(uint8_t new_prompt_u8) {
	prompt_u8 = new_prompt_u8;
}
//**********************************************************************

void set_Blank(uint8_t new_blank_u8) {
	blank_u8 = new_blank_u8;
}
//**********************************************************************

void BeepError2(void) {
	Beeper_12();	CipherPrint(0x12);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x13);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x11);	HAL_Delay(500);
}
//**********************************************************************

void BeepError3(void) {
	Beeper_12();	CipherPrint(0x12);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x13);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x12);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x13);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x12);	HAL_Delay(300);
	Beeper_12();	CipherPrint(0x11);	HAL_Delay(1000);
	CipherPrint(0x10);
}
//**********************************************************************

void BlankIndicatorStart (void) {
	HAL_TIM_Base_Start_IT(&htim3); // start TIM3 interupt
}
//**********************************************************************

void BlankIndicatorStop (void) {
	HAL_TIM_Base_Stop_IT(&htim3); // start TIM3 interupt
}
//**********************************************************************

void Prompt_Start(void) {
	HAL_TIM_Base_Start_IT(&htim4); // start TIM4 prompt
}

