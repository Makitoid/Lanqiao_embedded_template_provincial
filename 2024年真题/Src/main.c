//头文件
#include "main.h"
#include "rcc/bsp_rcc.h"
#include "basic/bsp_basic.h"
#include "LCD\bsp_lcd.h"
#include "tim\bsp_tim.h"

//执行函数
void KeyProc(void);
void LcdProc(void);
void LedProc(void);

//减速变量
__IO uint32_t uwTick_Key_Set;
__IO uint32_t uwTick_Led_Set;
__IO uint32_t uwTick_Lcd_Set;
__IO uint32_t uwTick_Detect_Set;
__IO uint32_t uwTick_3s_Set;
uint16_t uwTick_1000_Set;
uint8_t Flag_1000ms;

//Basic外设变量
uint8_t LedVal;
uint8_t ucKeyUp,ucKeyVal,ucKeyOld,unKeyDown;

//lcd变量
uint8_t LcdDisp[21];
uint8_t NullFlagA;
uint8_t NullFlagB;
//定时器变量
uint16_t PWM2_T_Origin;
uint16_t PWM3_T_Origin;
uint16_t PWM2_f_Origin;
uint16_t PWM3_f_Origin;
uint16_t PD=1000;
int16_t PX=0;
uint16_t PWM2_T_Final;
uint16_t PWM3_T_Final;
uint16_t PWM2_f_Final;
uint16_t PWM3_f_Final;
uint16_t PH=5000;
uint16_t NDA;
uint16_t NDB;
uint16_t NHA;
uint16_t NHB;
uint16_t PWM2_f_Max;
uint16_t PWM3_f_Max;
uint16_t PWM2_f_Min;
uint16_t PWM3_f_Min;

//控制变量
uint8_t Screen_Stat;
void RecdClear(void);
void Detect(void);
int main(void)
{
	HAL_Init();
	SystemClock_Config();
	//基本外设启动
	Basic_Init();
	//Lcd启动
	LCD_Init();
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	LCD_Clear(Black);
	//定时器启动
	PWM_Input_TIM2_Init();
	PWM_Input_TIM3_Init();
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
	while (1)
	{
		LedProc();
		KeyProc();
		LcdProc();
		Detect();
	}

}
void LcdProc(void)
{
	if(uwTick-uwTick_Lcd_Set<100) return;
	uwTick_Lcd_Set=uwTick;
	
	PWM2_f_Origin=1000000/PWM2_T_Origin;
	PWM3_f_Origin=1000000/PWM3_T_Origin;
	
	PWM2_f_Final=PWM2_f_Origin+PX;
	PWM3_f_Final=PWM3_f_Origin+PX;
	
	if(PWM2_f_Final<=0)
		PWM2_T_Final=0;
	else if(PWM2_f_Final>0)
		PWM2_T_Final=1000000/PWM2_f_Final;
			
	if(PWM3_f_Final<0)
		PWM3_T_Final=0;
	else if(PWM2_f_Final>0)
		PWM3_T_Final=1000000/PWM3_f_Final;
		
	
	PWM2_f_Origin=1/PWM2_T_Origin;
	if(Screen_Stat==0x00)
	{
		LedVal|= 0x01;
		sprintf((char *)LcdDisp,"        DATA");
		LCD_DisplayStringLine(Line2,LcdDisp);
		if(PWM2_f_Final<=0)
			sprintf((char *)LcdDisp,"     A=NULL");
		else if(PWM2_f_Final<1000)		
			sprintf((char *)LcdDisp,"     A=%dHz",PWM2_f_Final);					
		else if(PWM2_f_Final>=1000)
			sprintf((char *)LcdDisp,"     A=%.2fKHz",(float)PWM2_f_Final*0.001);
		LCD_DisplayStringLine(Line4,LcdDisp);
		
		
		if(PWM3_f_Final<=0)		
			sprintf((char *)LcdDisp,"     B=NULL");		
		else if(PWM3_f_Final<1000)		
			sprintf((char *)LcdDisp,"     B=%dHz",PWM3_f_Final);				
		else if(PWM3_f_Final>=1000)	
			sprintf((char *)LcdDisp,"     B=%.2fKHz",(float)PWM3_f_Final*0.001);
			
		LCD_DisplayStringLine(Line5,LcdDisp);
	}
	
	else if(Screen_Stat==0x01)
	{
		sprintf((char *)LcdDisp,"        DATA");
		LCD_DisplayStringLine(Line2,LcdDisp);
		if(PWM2_T_Final==0)		
			sprintf((char *)LcdDisp,"     A=NULL");					
		else if(PWM2_T_Final<1000)		
			sprintf((char *)LcdDisp,"     A=%dus",PWM2_T_Final);					
		else if(PWM2_T_Final>=1000)		
			sprintf((char *)LcdDisp,"     A=%.2fms",(float)PWM2_T_Final*0.001);
		LCD_DisplayStringLine(Line4,LcdDisp);		
		
		if(PWM3_T_Final==0)		
			sprintf((char *)LcdDisp,"     B=NULL");
		else if(PWM3_f_Final<1000)		
			sprintf((char *)LcdDisp,"     B=%dus",PWM3_T_Final);
		else if(PWM3_T_Final>=1000)		
			sprintf((char *)LcdDisp,"     B=%.2fms",(float)PWM3_T_Final*0.001);
		LCD_DisplayStringLine(Line5,LcdDisp);		
	}
	else if((Screen_Stat & (~0x0F)) == 0x10)
	{
		sprintf((char *)LcdDisp,"        PARA");
		LCD_DisplayStringLine(Line2,LcdDisp);
		
		sprintf((char *)LcdDisp,"     PD=%dHz",PD);
		LCD_DisplayStringLine(Line4,LcdDisp);
		sprintf((char *)LcdDisp,"     PH=%dHz",PH);
		LCD_DisplayStringLine(Line5,LcdDisp);
		sprintf((char *)LcdDisp,"     PX=%dHz",PX);
		LCD_DisplayStringLine(Line6,LcdDisp);
		
	}
	else if(Screen_Stat==0x20)
	{
		sprintf((char *)LcdDisp,"        RECD");
		LCD_DisplayStringLine(Line2,LcdDisp);
		
		sprintf((char *)LcdDisp,"     NDA=%d",NDA);
		LCD_DisplayStringLine(Line4,LcdDisp);
		sprintf((char *)LcdDisp,"     NDB=%d",NDB);
		LCD_DisplayStringLine(Line5,LcdDisp);
		sprintf((char *)LcdDisp,"     NHA=%d",NHA);
		LCD_DisplayStringLine(Line6,LcdDisp);
		sprintf((char *)LcdDisp,"     NHB=%d",NHB);
		LCD_DisplayStringLine(Line7,LcdDisp);
	}
}

void KeyProc(void)
{
	if(uwTick-uwTick_Key_Set<50) return;
	uwTick_Key_Set=uwTick;
	
	ucKeyVal=KeyScan();
	unKeyDown=ucKeyVal&(ucKeyVal^ucKeyOld);
	ucKeyUp=~ucKeyVal&(ucKeyVal^ucKeyOld);
	ucKeyOld=ucKeyVal;
	
	if(unKeyDown==4)
	{
		if(Screen_Stat==0x01|Screen_Stat==0x00)
		{
			Screen_Stat=0x10;
			LedVal&=(~0x01);
		}
		else if(Screen_Stat==0x10)
		{
			Screen_Stat=0x20;
			LedVal&=(~0x01);
		}
		else if (Screen_Stat==0x20)
			Screen_Stat=0x00;
		LCD_Clear(Black);
	}
	if(unKeyDown==3)
	{
		uwTick_1000_Set=uwTick;
	}	
	if(ucKeyUp==3)
	{
		if(uwTick-uwTick_1000_Set<400)
		{
			if(Screen_Stat==0x00)
				Screen_Stat=0x01;
			else if(Screen_Stat==0x01)
				Screen_Stat=0x00;
			
			if(Screen_Stat==0x10)
				Screen_Stat=0x11;
			else if(Screen_Stat==0x11)
				Screen_Stat=0x12;
			else if(Screen_Stat==0x12)
				Screen_Stat=0x10;
			LCD_Clear(Black);
		}
		else
		{
			if(Screen_Stat==0x20)
				RecdClear();
		}
	}
	if(unKeyDown==1)
	{
		if(Screen_Stat==0x10)
		{
			PD+=100;
			if(PD>=1000)
				PD=1000;
			LCD_ClearLine(Line4);
		}
		else if(Screen_Stat==0x11)
		{
			PH+=100;
			if(PD>=10000)
				PD=10000;
			LCD_ClearLine(Line5);
		}
		else if(Screen_Stat==0x12)
		{
			PX+=100;
			if(PX>=1000)
				PX=1000;
			LCD_ClearLine(Line6);			
		}
	}
	if(unKeyDown==2)
	{
		if(Screen_Stat==0x10)
		{
			PD-=100;
			if(PD<=100)
				PD=100;	
			LCD_ClearLine(Line4);			
		}
		else if(Screen_Stat==0x11)
		{
			PH-=100;
			if(PD<=1000)
				PD=1000;
			LCD_ClearLine(Line5);
		}
		else if(Screen_Stat==0x12)
		{
			PX-=100;
			if(PX<=(-1000))
				PX=-1000;
			LCD_ClearLine(Line6);
		}
	}
}

void LedProc(void)
{
	if(uwTick-uwTick_Led_Set<200) return;
	uwTick_Led_Set=uwTick;
	LedDisp(LedVal);
	
	if(NDA>=3 ||NDB>=3)
		LedVal|=0x80;
	else 
		LedVal&=(~0x80);
}

void RecdClear(void)
{
	NDA=0;
	NDB=0;
	NHA=0;
	NHB=0;
}

void Detect(void)
{
	if(uwTick-uwTick_Detect_Set<10) return;
	uwTick_Detect_Set=uwTick;
	if(PWM2_f_Final>PH)
	{
		NDA++;
		LedVal|=0x02;
	}
	else
		LedVal&=(~0x02);
	if(PWM3_f_Final>PH)
	{
		NDB++;
		LedVal|=0x03;
	}
	else 
		LedVal&=(~0x03);
	
	if(uwTick-uwTick_3s_Set<3000)
	{
		if(PWM3_f_Final > PWM3_f_Max)
			PWM3_f_Max = PWM3_f_Final;
		if(PWM3_f_Final < PWM3_f_Min)
			PWM3_f_Min = PWM3_f_Final;		
		
		if(PWM2_f_Final > PWM2_f_Max)
			PWM2_f_Max = PWM3_f_Final;
		if(PWM3_f_Final < PWM2_f_Min)
			PWM2_f_Min = PWM2_f_Final;	
	}
	else
	{
		if((PWM3_f_Max - PWM3_f_Min) > PD)
			NDB++;
		if((PWM2_f_Max - PWM2_f_Min) > PD)
			NDA++;		
		
		uwTick_3s_Set = uwTick;
    PWM2_f_Max = 500;
    PWM2_f_Min = 23000; 
    PWM3_f_Max = 500; 
    PWM3_f_Min = 23000;	
	}
	
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
			PWM2_T_Origin=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
	}
	if(htim->Instance==TIM3)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
			PWM2_T_Origin=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
	}
}