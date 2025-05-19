/*
 * main.c
 */
#include <msp430f6638.h>

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 //关闭看门狗
  P1DIR |= BIT0;                            //设置P1.0口方向为输出
  P1SEL |= BIT0;                            //设置P1.0功能为外设

  __bis_SR_register(LPM3_bits + GIE);       //进入低功耗模式3 开中断
  // __bis_SR_register(LPM4_bits + GIE);         //进入低功耗模式4 开中断
  __no_operation();                         //空操作
}
