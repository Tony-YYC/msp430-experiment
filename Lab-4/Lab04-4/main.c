/*
 * main.c
 */
#include <msp430f6638.h>
#include <stdint.h>
#define MCLK_FREQ 16000000
#define FEED_DOG // 定义此标志则喂狗，注释掉就不喂狗

typedef struct // 以指针形式定义P8口的各个寄存器
{
    const volatile uint8_t* PxIN; // 定义一个不会被编译的无符号字符型变量
    volatile uint8_t* PxOUT;
    volatile uint8_t* PxDIR;
    volatile uint8_t* PxREN;
    volatile uint8_t* PxSEL;
} GPIO_TypeDef;

const GPIO_TypeDef GPIO4 = { &P4IN, &P4OUT, &P4DIR, &P4REN, &P4SEL };

const GPIO_TypeDef GPIO5 = { &P5IN, &P5OUT, &P5DIR, &P5REN, &P5SEL };

const GPIO_TypeDef GPIO8 = { &P8IN, &P8OUT, &P8DIR, &P8REN, &P8SEL };

const GPIO_TypeDef* LED_GPIO[5] = { &GPIO4, &GPIO4, &GPIO4, &GPIO5, &GPIO8 };
const uint8_t LED_PORT[5] = { BIT5, BIT6, BIT7, BIT7, BIT0 };

void start_indicator();

void main(void) {
    WDTCTL = WDT_ARST_1000; //使用ACLK做时钟，看门狗模式，重置时间设置为1000ms

    int i;
    for (i = 0; i < 5; ++i)
        *LED_GPIO[i]->PxDIR |= LED_PORT[i]; // 设置各LED灯所在端口为输出方向

    start_indicator(); //指示系统重启

#ifdef FEED_DOG
    while (1) {
        WDTCTL = WDTPW + WDTCNTCL; //喂狗
        __delay_cycles(MCLK_FREQ / 10); //延时100ms
    }
#endif
    __bis_SR_register(LPM0_bits + GIE); //进入低功耗模式3 开中断
}

// 该函数依次点亮五盏LED灯，然后再全灭，指示系统重启
void start_indicator() {
    int i;
    for (i = 0; i < 2; ++i) {
        *LED_GPIO[i]->PxOUT |= LED_PORT[i];
        __delay_cycles(MCLK_FREQ / 10);
        WDTCTL = WDTPW + WDTCNTCL; //喂狗
    }
    __delay_cycles(MCLK_FREQ / 20);
    for (i = 0; i < 2; ++i) {
        *LED_GPIO[i]->PxOUT &= ~LED_PORT[i];
    }
}
