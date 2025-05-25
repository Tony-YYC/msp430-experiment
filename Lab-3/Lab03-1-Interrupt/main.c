/*
 * main.c
 */
#include <msp430f6638.h>
#define XT1CLK_FREQ 32760

void init_clock();

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; //关闭看门狗
    init_clock();
    P4DIR |= BIT7; //设置P4.7口方向为输出
    P4DIR &= ~BIT0;
    P4REN |= BIT0; //使能P4.0上拉电阻
    P4OUT |= BIT0; //P4.0口置高电平
    P4IES |= BIT0; //中断沿设置（下降沿触发）
    P4IFG &= ~BIT0; //清P4.0中断标志
    P4IE |= BIT0; //使能P4.0口中断

    __bis_SR_register(LPM3_bits + GIE); //进入低功耗模式3 开中断
    __no_operation(); //空操作
}

// P4中断函数
#pragma vector = PORT4_VECTOR
__interrupt void Port_4(void) {
    if (P4IFG & BIT0) {
        P4IE &= ~BIT0; //关闭P4.0中断
        P4OUT |= BIT7; //开LED7灯
        __delay_cycles(XT1CLK_FREQ); //延时1s
        P4OUT &= ~BIT7; //关LED7灯
        P4IFG &= ~BIT0; //清P4.0中断标志位
        P4IE |= BIT0; //重新使能P4.0口中断
    } else {
        P4IFG = 0; //清中断标志位
    }
}

void init_clock()
{
    while(BAKCTL & LOCKIO) // Unlock XT1 pins for operation
        BAKCTL &= ~(LOCKIO);
    UCSCTL6 &= ~XT1OFF; //启动XT1
    while (UCSCTL7 & XT1LFOFFG) //等待XT1稳定
        UCSCTL7 &= ~(XT1LFOFFG);

    UCSCTL4 = SELA__XT1CLK + SELS__XT1CLK + SELM__XT1CLK; //时钟设为XT1，频率较低，方便软件延时
}
