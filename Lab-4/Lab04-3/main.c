/*
 * main.c
 */
#include <msp430f6638.h>
#include <stdint.h>

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

uint8_t led_count = 0; // 当前点亮的LED数量
const uint8_t DEBOUNCE_THRESHOLD = 5;

void init_timer();
void refresh_led();
void toggle_led();

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // 关闭看门狗

    int i;
    for (i = 0; i < 5; ++i)
        *LED_GPIO[i]->PxDIR |= LED_PORT[i]; // 设置各LED灯所在端口为输出方向

    init_timer();

    //注意：P4.4口为按键S3，P4.3口为按键S4
    P4DIR &= (~BIT4 & ~BIT3);
    P4REN |= (BIT4 + BIT3); //使能P4.4和P4.3的上拉电阻
    P4OUT |= (BIT4 + BIT3); //P4.4和P4.3口置高电平
    P4IES |= (BIT4 + BIT3); //中断沿设置（下降沿触发）
    P4IFG &= (~BIT4 & ~BIT3); //清P4.4和P4.3中断标志
    P4IE |= (BIT4 + BIT3); //使能P4.4和P4.3口中断

    __bis_SR_register(LPM0_bits + GIE); // 进入低功耗并开启总中断
}

void init_timer() {
    TA0CTL |= MC_1 + TASSEL_2 + TACLR;
    // 时钟为SMCLK,比较模式，开始时清零计数器
    TA0CCTL0 = CCIE; // 比较器中断使能
    TA0CCR0 = 1000000; // 比较值设为1,000,000，相当于1s的时间间隔
}

void refresh_led() {
    int i;
    for (i = 0; i < 5; i++) {
        if (i < led_count)
            *LED_GPIO[i]->PxOUT |= LED_PORT[i];
        else
            *LED_GPIO[i]->PxOUT &= ~LED_PORT[i];
    }
}

void toggle_led() {
    int i;
    for (i = 0; i < led_count; i++)
        *LED_GPIO[i]->PxOUT ^= LED_PORT[i];
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    toggle_led();
}

#pragma vector = PORT4_VECTOR
__interrupt void Port_4(void) {
    int debounce_count = 0;
    if (P4IFG & BIT4) {
        P4IE &= ~BIT4; //关P4.3口中断
        while (debounce_count < DEBOUNCE_THRESHOLD) {
            if ((P4IN & BIT4) == 0) {
                debounce_count++;
                __delay_cycles(200);
            }
        }
        if (led_count < 5) {
            led_count++;
        }
        refresh_led();
        init_timer();
        P4IFG &= ~BIT4;
        P4IE |= BIT4; //重新开P4.3口中断
    } else if (P4IFG & BIT3) {
        P4IE &= ~BIT3; //关P4.3口中断
        while (debounce_count < DEBOUNCE_THRESHOLD) {
            if ((P4IN & BIT4) == 0) {
                debounce_count++;
                __delay_cycles(200);
            }
        }
        led_count = 0;
        refresh_led();
        P4IFG &= ~BIT3;
        P4IE |= BIT3; //重新开P4.3口中断
    } else {
        P4IFG = 0;
    }
}
