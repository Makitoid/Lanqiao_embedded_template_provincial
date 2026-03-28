//头文件
#include "main.h"
#include "rcc/bsp_rcc.h"
#include "key_led/bsp_key_led.h"
#include "lcd/bsp_lcd.h"
#include "usart/bsp_uart1.h"
#include "i2c/bsp_i2c.h"
#include "adc/bsp_adc.h"
#include "tim/bsp_tim.h"
#include "rtc/bsp_rtc.h"

//减速变量
__IO uint32_t uwTick_led_set;
__IO uint32_t uwTick_key_set;
__IO uint32_t uwTick_lcd_set;
__IO uint32_t uwTick_uart1_set;

//执行函数
void Led_proc(void);
void Key_proc(void);
void Lcd_proc(void);
void Uart1_proc(void);

//led变量
uint8_t ledval;

//key变量
uint8_t ucKey_val,ucKey_old,ucKey_up,unKey_down;

//lcd变量
uint8_t lcd_disp[21];

//UART1变量
uint32_t Rx_buffer;
uint16_t counter;
uint8_t str[40];

//I2C变量
uint8_t EEPROM_test_1[5]={0x11,0x22,0x33,0x44,0x55};
uint8_t EEPROM_test_2[5]={0};

//RES4017可编程电阻变量
uint8_t RES_4017;

//PWM输入变量
uint16_t PWM_T_Count;
uint16_t PWM_D_Count;
float PWM_Duty;

//RTC变量
RTC_TimeTypeDef H_M_S;
RTC_DateTypeDef Y_M_D;

int main(void)
{
  //基础启动
  HAL_Init();
  SystemClock_Config();
  //key、led启动
  Key_led_Init();
  //LCD启动
  LCD_Init();
  LCD_SetTextColor(Red);
  LCD_SetBackColor(Green);
  LCD_Clear(Green);
  //Uart1启动
  Uart1_Init();
  //打开串口
  HAL_UART_Receive_IT(&huart1,(uint8_t *)&Rx_buffer,1);
  //I2C调试
  I2CInit();
  i2c_24C02_write(EEPROM_test_1,0,5);
  HAL_Delay(1);
  i2c_24C02_read(EEPROM_test_2,0,5);
  //4017测试
  write_resistor(0x77);
  RES_4017=read_resistor();
  //ADC读取
  ADC1_Init();
  ADC2_Init();
  //定时器打开
  Basic_TIM6_Init();
  PWM_Input_TIM2_Init();
  SQU_Output_TIM15_Init();
  PWM_Output_TIM3_Init();
  PWM_Output_TIM17_Init();
  //TIM6中断打开
  HAL_TIM_Base_Start_IT(&htim6);
  //TIM2初始化
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
  //TIM3/17引脚打开
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
  //TIM15初始化
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
	if(uwTick-uwTick_lcd_set<100) return;
	uwTick_lcd_set=uwTick;
	
	sprintf((char *)lcd_disp,"   Hello Mskitoid");
	LCD_DisplayStringLine(Line0, lcd_disp);
	
	sprintf((char *)lcd_disp,"EE:%x%x%x%x%x R:%x",EEPROM_test_2[0],EEPROM_test_2[1],EEPROM_test_2[2],EEPROM_test_2[3],
			EEPROM_test_2[4],RES_4017);
	LCD_DisplayStringLine(Line1, lcd_disp);
	
	sprintf((char *)lcd_disp,"ADC1:%6.3fV",((((float)(getADC1()))/4096)*3.3));
	LCD_DisplayStringLine(Line2, lcd_disp);
	sprintf((char *)lcd_disp,"ADC2:%6.3fV",((((float)(getADC2()))/4096)*3.3));
	LCD_DisplayStringLine(Line3, lcd_disp);
	
	sprintf((char *)lcd_disp,"R40:%04d,%:%4.1f%%",(unsigned int)(10000000/PWM_T_Count),PWM_Duty*100);
	LCD_DisplayStringLine(Line4, lcd_disp);
	
	HAL_RTC_GetDate(&hrtc,&Y_M_D,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc,&H_M_S,RTC_FORMAT_BIN);
	sprintf((char *)lcd_disp,"%02d-%02d-%02d",(unsigned int)H_M_S.Hours,(unsigned int)H_M_S.Minutes,(unsigned int)H_M_S.Seconds);
	LCD_DisplayStringLine(Line6, lcd_disp);
}

void Led_proc(void)
{
	if(uwTick-uwTick_led_set<200) return;
	uwTick_led_set=uwTick;
	
	Led_disp(ledval);
}
	
void Key_proc(void)
{
	if(uwTick-uwTick_key_set<50) return;
	uwTick_key_set=uwTick;
	
	ucKey_val=Key_scan();
	ucKey_up=ucKey_val&(ucKey_val^ucKey_old);
	unKey_down=~ucKey_val&(ucKey_val^ucKey_old);
	ucKey_old=ucKey_val;
	
	if(unKey_down==4)
		ledval=0x88;
	if(unKey_down==3)
		ledval=0x00;
}

void Uart1_proc(void)
{
//	if(uwTick-uwTick_uart1_set<500) return;
//	uwTick_uart1_set=uwTick;
//	
//	if(++counter==30)
//		counter=0;
//	sprintf(str,"%03d:Hello world!\r\n",counter);
//	HAL_UART_Transmit(&huart1,str,strlen(str),50);
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if(uwTick-uwTick_uart1_set<300) return;
//   uwTick_uart1_set=uwTick;
//    
//    if(++counter==30)
//    	counter=0;
//    sprintf(str,"%03d:Hello world!\r\n",counter);
//	HAL_UART_Receive_IT(&huart1,(uint8_t *)Rx_buffer,1);
//    HAL_UART_Transmit(&huart1,str,strlen(str),50);
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM6)
	{
		if(++counter==10)
		{
			counter=0;
			sprintf(str,"Hello world!\r\n");
			HAL_UART_Transmit(&huart1,(unsigned char *)str,strlen(str),50);
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM_T_Count=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
			PWM_Duty=(float)PWM_D_Count/PWM_T_Count;
		}
		else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
			PWM_D_Count=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2);
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM15)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_1,__HAL_TIM_GetCounter(htim)+500);
	}
}