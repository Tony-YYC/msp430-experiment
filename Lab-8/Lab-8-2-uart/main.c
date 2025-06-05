/*
 * main.c
 */
// Define target MCLK and XT2 crystal frequencies if using the provided init_clock()
// These are example values, adjust them to your hardware.
#include "uart_lib.h"
#define MCLK_FREQ 16000000UL // Example: Target MCLK at 16MHz
#define XT2_FREQ 4000000UL // Example: XT2 crystal at 4MHz

#include "dr_tft.h"
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>

unsigned char flag0 = 0, flag1 = 0;
uint16_t sx = 10, sy = 20; //TFT屏显示位置
unsigned char received_byte[] = { '0', '\0' };

void TimerA_Init(void); //定时器TA初始化函数

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; //关闭看门狗

    uart_init(BAUD_9600); //初始化UART1，波特率9600
    initTFT(); //初始化TFT屏幕
    etft_AreaSet(0, 0, 319, 239, 0); //TFT清屏
    TimerA_Init(); //初始化定时器
    _EINT(); //开启中断

    uart_write_buffer((const uint8_t*)"UART Library Initialized. Echoing characters...\r\n", 47);
    etft_DisplayString("Recv Data From UART (The data will be echoed back): ",
                       sx,
                       sy,
                       65535,
                       0); //TFT屏显示接收数据的标识
    sy += 20;

    while (1) {
        // Check if there's data available in the receive buffer
        if (uart_available() > 0) {
            // Read the byte from the buffer
            if (uart_read_byte(received_byte)) {
                // Echo the byte back
                uart_write_byte(received_byte[0]);
                etft_DisplayString((const char *)received_byte, sx, sy, 65535, 0);
                sx += 8;
                if (sx + 10 > TFT_XSIZE) {
                    sx = 10; // Reset x position if it exceeds screen width
                    sy += 20; // Move to next line
                }

                if (received_byte[0] == '\n')
                { // 记得串口工具使用LF换行符，不要使用CRLF换行符，windows环境尤其注意！
                    uart_write_buffer((const uint8_t*)"\nLine End Received.\n", 20);
                    sx = 10;
                    sy += 20;
                }
            }
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR //定时器TA中断服务函数
__interrupt void Timer_A(void) {
    static unsigned char i = 0;
    i++;
    if (i >= 20) //记满二十次为1s
    {
        i = 0;
        flag0 = 1; //改变标识数据的值
    }
}

void TimerA_Init(void) //定时器TA初始化函数
{
    TA0CTL |= MC_1 + TASSEL_2 + TACLR; //时钟为SMCLK,比较模式，开始时清零计数器
    TA0CCTL0 = CCIE; //比较器中断使能
    TA0CCR0 = 50000; //比较值设为50000，相当于50ms的时间间隔
}
