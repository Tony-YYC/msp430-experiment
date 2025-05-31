/*
 * main.c
 */
#include <math.h>
#include <msp430f6638.h>
#include <stdint.h>

// #define POINTS 10
#define M_PI 3.14159265358979323846f // 定义圆周率
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

// unsigned int sinetable[POINTS] = { 2048, 3261, 3976, 3976, 3261, 2048, 835, 120, 120, 835 };
unsigned char index = 0;
void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // 关闭看门狗
    initClock(); // 配置系统时钟
    P7DIR |= BIT6; // 设置P7.6口为输出口
    P7SEL |= BIT6; // 使能P7.6口第二功能位
    DAC12_0CTL0 |= DAC12IR; // 设置参考电压满刻度值，使Vout = Vref×(DAC12_xDAT/4096)
    DAC12_0CTL0 |= DAC12SREF_1; // 设置参考电压为AVCC
    DAC12_0CTL0 |= DAC12AMP_5; // 设置运算放大器输入输出缓冲器为中速中电流
    DAC12_0CTL0 |= DAC12CALON; // 启动校验功能
    DAC12_0CTL0 |= DAC12OPS; // 选择第二通道P7.6
    DAC12_0CTL0 |= DAC12ENC; // 转化使能
    DAC12_0DAT = 0xFFF; // 输入数据
    TA0CCTL0 = CCIE; // 设置定时器中断
    TA0CCR0 = 4000; // 设置定时器中断时间
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // 设置定时器
    __bis_SR_register(LPM0_bits + GIE); // 进入低功耗状态
    while (1) {
    };
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    float amplitude =
        (1.0f + sinf((float)index / 100.0f * 2 * M_PI)) * 2000.0f; // 计算当前索引对应的正弦波值
    uint16_t value = (uint16_t)amplitude; // 将正弦波值转换为DAC12的输入值
    DAC12_0DAT = value; // 输出正弦波表中的值
    index = (index + 1) % 100; // 循环索引
}