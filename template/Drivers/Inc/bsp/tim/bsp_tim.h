#include "main.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim17;

void Basic_TIM6_Init(void);
void PWM_Input_TIM2_Init(void);
void PWM_Output_TIM3_Init(void);
void PWM_Output_TIM17_Init(void);
void SQU_Output_TIM15_Init(void);
