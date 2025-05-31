/*
 * main.c
 */

#include "dr_lcdseg.h" //调用段式液晶驱动头文件
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define XT2_FREQ 4000000
#define MCLK_FREQ 16000000
#define SMCLK_FREQ 4000000

void initClock() {
    while (BAKCTL & LOCKIO) // 解锁XT1引脚操作
        BAKCTL &= ~(LOCKIO);
    UCSCTL6 &= ~XT1OFF; // 启动XT1，选择内部时钟源
    P7SEL |= BIT2 + BIT3; // XT2引脚功能选择
    UCSCTL6 &= ~XT2OFF; // 启动XT2
    while (SFRIFG1 & OFIFG) // 等待XT1、XT2与DCO稳定
    {
        UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG);
        SFRIFG1 &= ~OFIFG;
    }
    UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; // 避免DCO调整中跑飞
    UCSCTL1 = DCORSEL_5; // 6000kHz~23.7MHz
    UCSCTL2 = MCLK_FREQ / (XT2_FREQ / 16); // XT2频率较高，分频后作为基准可获得更高的精度
    UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16; // XT2进行16分频后作为基准
    while (SFRIFG1 & OFIFG) // 等待XT1、XT2与DCO稳定
    {
        UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG);
        SFRIFG1 &= ~OFIFG;
    }
    UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1; // 设定几个CLK的分频
    UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__DCOCLK; // 设定几个CLK的时钟源
}
void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // 关闭看门狗
    initClock(); // 配置系统时钟
    initLcdSeg(); // 初始化段式液晶
    ADC12CTL0 |= ADC12MSC; // 自动循环采样转换
    ADC12CTL0 |= ADC12ON; // 启动ADC12模块
    ADC12CTL1 |= ADC12CONSEQ1; // 选择单通道循环采样转换
    ADC12CTL1 |= ADC12SHP; // 采样保持模式
    ADC12MCTL0 |= ADC12INCH_15; // 选择通道15，连接拨码电位器
    ADC12CTL0 |= ADC12ENC;
    volatile unsigned int value = 0; // 设置判断变量
    float result = 0.0f; // 设置结果变量
    while (1) {
        ADC12CTL0 |= ADC12SC; // 开始采样转换
        __delay_cycles(1000);
        value = ADC12MEM0; // 把结果赋给变量
        unsigned int i;
        for (i = 0; i < 6; i++)
            LCDSEG_SetDigit(i, -1); // 段式液晶清屏
        result = (float)value * 3.3f / 4096.0f; // 将ADC值转换为电压
        LCDSEG_DisplayFloatNum(result, 3); // 显示结果
        __delay_cycles(MCLK_FREQ / 2); // 延时500ms
    }
}
