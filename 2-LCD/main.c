/*
 * main.c
 */
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dr_lcdseg.h"   //调用段式液晶驱动头文件

#define XT2_FREQ   4000000

#define MCLK_FREQ 16000000
#define SMCLK_FREQ 4000000

void initClock()
{
  while(BAKCTL & LOCKIO) //解锁XT1引脚操作
BAKCTL &= ~(LOCKIO);
UCSCTL6 &= ~XT1OFF; //启动XT1，选择内部时钟源
    P7SEL |= BIT2 + BIT3; //XT2引脚功能选择
UCSCTL6 &= ~XT2OFF; //启动XT2
while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
{
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
    }
  UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; //避免DCO调整中跑飞
  UCSCTL1 = DCORSEL_5; //6000kHz~23.7MHz
  UCSCTL2 = MCLK_FREQ / (XT2_FREQ / 16); //XT2频率较高，分频后作为基准可获得更高的精度
  UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16; //XT2进行16分频后作为基准
  while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
{
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
  }
  UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1; //设定几个CLK的分频
  UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__DCOCLK; //设定几个CLK的时钟源
}

void displaySJTU() {
	int i;
	for(i=3;i<=5;++i)
	    LCDSEG_ResetSpecSymbol(i);
    LCDSEG_SetDigit(3, CHAR_S);
    LCDSEG_SetDigit(2, CHAR_J);
    LCDSEG_SetDigit(1, CHAR_T);
    LCDSEG_SetDigit(0, CHAR_U);
    LCDSEG_SetDigit(4, -1);
    LCDSEG_SetDigit(5, -1);
}

void main(void)
{
	unsigned char i,num1;
	int32_t num2;
    WDTCTL = WDTPW | WDTHOLD;	// 停止看门狗
    initClock();             //配置系统时钟
    initLcdSeg();           //初始化段式液晶
    while(1)
    {
       // 新增：显示SJTU并保持2秒
       displaySJTU();
       __delay_cycles(MCLK_FREQ * 2); // 延时2000ms

       char num [8] = "0305.24";
       LCDSEG_DisplayNumString(num);   //显示六位数，从111111-999999
       __delay_cycles(MCLK_FREQ * 2);	//延时2000ms

		for(i=0;i<6;i++)
			LCDSEG_SetDigit(i,-1);     //段式液晶清屏
      	__delay_cycles(MCLK_FREQ / 10); //延时100ms
    }
}
