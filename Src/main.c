//声明
#include "main.h"
#include "rcc/bsp_rcc.h"
#include "key_led/bsp_key_led.h"
#include "lcd/lcd.h"
#include "usart/bsp_uart1.h"
#include "i2c/bsp_i2c.h"
#include "adc/bsp_adc.h"
#include "tim/bsp_tim.h"
#include "rtc/bsp_rtc.h"

//执行函数
void Key_proc(void);
void Led_proc(void);
void Lcd_proc(void);
void Uart1_proc(void);

//减速变量
__IO uint32_t uwTick_key_point;
__IO uint32_t uwTick_led_point;
__IO uint32_t uwTick_lcd_point;
__IO uint32_t uwTick_Uart1_point;

//led变量
uint8_t ledval;

//key变量
uint8_t ucKey_val,unKey_down,ucKey_up,ucKey_old;

//lcd
uint8_t Lcd_disp[21];

//uart1
uint8_t str[40];
uint32_t Rx_buffer;
uint16_t counter;

//i2c
uint8_t EEPROM_num_1[5]={0x11,0x22,0x33,0x44,0x55};
uint8_t EEPROM_num_2[5]={0};

//res4017可编程电阻
uint8_t res_4017;

//pwm测量
uint16_t PWM_T_count;
uint16_t PWM_D_count;
float PWM_Duty;

//rtc

RTC_TimeTypeDef H_M_S_Time;
RTC_DateTypeDef Y_M_D_Date;

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	Key_led_Init();
	LCD_Init();
	LCD_SetTextColor(Red);
	LCD_SetBackColor(Green);
	LCD_Clear(Green);
	Uart1_Init();
	HAL_UART_Receive_IT(&huart1,(uint8_t *)&Rx_buffer,1);
	I2CInit();
	i2c_24C02_write(EEPROM_num_1,0,5);
	HAL_Delay(1);
	i2c_24C02_read(EEPROM_num_2,0,5);
	res4017_write(0x77);
	res_4017=res4017_read();
	ADC1_Init();
	ADC2_Init();
	Basic_TIM6_Init();
	HAL_TIM_Base_Start_IT(&htim6);
	PWM_Input_TIM2_Init();
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
	PWM_Output_TIM3_Init();
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	PWM_Output_TIM17_Init();
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	SQU_Output_TIM15_Init();
	HAL_TIM_OC_Start_IT(&htim15,TIM_CHANNEL_1);
	while (1)
	{
		Key_proc();
		Led_proc();	
		Lcd_proc();
		Uart1_proc();
	}
 
}

void Lcd_proc(void)
{
	if(uwTick-uwTick_led_point<200)  return;
	uwTick_led_point=uwTick;
	
	sprintf((char *)Lcd_disp,"Hello Makitoid");
	LCD_DisplayStringLine(Line1, Lcd_disp);
	
	sprintf((char *)Lcd_disp,"EE:%x%x%x%x%x,R:%x",EEPROM_num_2[0],EEPROM_num_2[1],EEPROM_num_2[2],EEPROM_num_2[3],
					EEPROM_num_2[4],res_4017);
	LCD_DisplayStringLine(Line2, Lcd_disp);
	
	sprintf((char *)Lcd_disp,"R15:%6.3f",((((float)getADC1())/4096)*3.3));
	LCD_DisplayStringLine(Line3,Lcd_disp);
	
	sprintf((char *)Lcd_disp,"R17:%6.3f",((((float)getADC2())/4096)*3.3));
	LCD_DisplayStringLine(Line4,Lcd_disp);
	
	sprintf((char *)Lcd_disp,"R40:%05dHZ,%4.1f%%",(unsigned int)(1000000/PWM_T_count),PWM_Duty*100);
	LCD_DisplayStringLine(Line5,Lcd_disp);
	
	HAL_RTC_GetTime(&hrtc,&H_M_S_Time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&Y_M_D_Date,RTC_FORMAT_BIN);
	sprintf((char *)Lcd_disp, "Time:%02d-%02d-%02d",(unsigned int)H_M_S_Time.Hours,(unsigned int)H_M_S_Time.Minutes,(unsigned int)H_M_S_Time.Seconds);
	LCD_DisplayStringLine(Line6,Lcd_disp);
}

void Led_proc(void)
{
	if(uwTick-uwTick_led_point<200)  return;
	uwTick_led_point=uwTick;
	
	Led_disp(ledval);
}

void Key_proc(void)
{
	if(uwTick-uwTick_key_point<50)  return;
	uwTick_key_point=uwTick;
	
	ucKey_val=Key_scan();
	unKey_down=ucKey_val&(ucKey_val^ucKey_old);
	ucKey_up=~ucKey_val&(ucKey_val^ucKey_old);
	ucKey_old=ucKey_val;
	
	if(unKey_down==3)
		ledval=0x88;
	if(unKey_down==4)
		ledval=0x00;
}

void Uart1_proc(void)
{
//	if(uwTick-uwTick_Uart1_point<200)  return;
//	uwTick_Uart1_point=uwTick;
//	
//	if(counter++==30)
//		counter=0;
//	sprintf(str,"%02dtest\r\n",counter);
//	HAL_UART_Transmit(&huart1,(unsigned char *)str,strlen(str),50);
	
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if(counter++==30)
//		counter=0;
//	HAL_UART_Receive_IT(&huart1,(uint8_t *)&Rx_buffer,1);
//	sprintf(str,"%02dtest\r\n",counter);
//	HAL_UART_Transmit(&huart1,(unsigned char *)str,strlen(str),50);
//	
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM6)
		if(counter++==10)
		{
			counter=0;
			sprintf((char *)str,"Hell0?");
			HAL_UART_Transmit(&huart1,str,strlen(str),50);
		}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM_T_count=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
			PWM_Duty=(float)PWM_D_count/PWM_T_count;
		}
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
			PWM_D_count=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM15)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_1,(__HAL_TIM_GetCounter(htim)+500));
	}
}
