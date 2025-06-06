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
uint8_t received_byte[] = { '0', '\0' };

void TimerA_Init(void); //定时器TA初始化函数
void init_clock();

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; //关闭看门狗

    init_clock();
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
    sy += 32;

    while (1) {
        // Check if there's data available in the receive buffer
        if (uart_available() > 0) {
            // Read the byte from the buffer
            if (uart_read_byte(received_byte)) {
                // Echo the byte back
                uart_write_byte(received_byte[0]);
                etft_DisplayString((const char *)received_byte, sx, sy, 65535, 0);
                sx += 8;
                if (sx + 10 > TFT_YSIZE) {
                    sx = 10; // Reset x position if it exceeds screen width
                    sy += 16; // Move to next line
                }

                if (received_byte[0] == '\n')
                { // 记得串口工具使用LF换行符，不要使用CRLF换行符，windows环境尤其注意！
                    uart_write_buffer((const uint8_t*)"Line End Received.\n", 19);
                    sx = 10;
                    sy += 16;
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

void init_clock() {
    // This is the clock initialization from your example.
    // Ensure XT1_FREQ (if used) and XT2_FREQ are defined and match your hardware.
    // For simplicity, if you don't have external crystals, you might configure
    // MCLK and SMCLK from DCO only. This example uses the more complex setup.

    while (
        BAKCTL
        & LOCKIO) // Unlock XT1 pins for operation [Not directly in provided manual but common for F6xxx]
        BAKCTL &= ~(LOCKIO);
    UCSCTL6 &= ~XT1OFF; // Enable XT1 [Not directly in provided manual but common for F6xxx]

    P7SEL |= BIT2 + BIT3; // Select XT2 pins
    UCSCTL6 &= ~XT2OFF; // Enable XT2

    // Wait for XT1, XT2, and DCO to stabilize
    // It's generally better to check individual flags (XT1LFOFFG, XT2OFFG, DCOFFG)
    do {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG); // Clear DCO, XT1, XT2 fault flags
        SFRIFG1 &= ~OFIFG; // Clear fault interrupt flag
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag

    // Configure DCO
    // Assuming MCLK_FREQ and XT2_FREQ are defined globally (e.g., 8MHz MCLK, 4MHz XT2)
    UCSCTL4 = SELA__XT1CLK | SELS__XT2CLK
        | SELM__XT2CLK; // Temporarily set MCLK to XT2 to avoid issues during DCO config
    UCSCTL1 = DCORSEL_5; // Select DCO range for MCLK_FREQ (e.g., 6MHz to 23.7MHz for DCORSEL_5)
    UCSCTL2 = (MCLK_FREQ / (XT2_FREQ / 16)) - 1; // FLLN = (MCLK_FREQ / (XT2_FREQ / FLLREFDIV)) - 1
    // Ensure FLLREFDIV below matches.
    // Target DCO = (N+1) * FLLRef
    // N = UCSCTL2, FLLRef = REFCLK / FLLREFDIV
    UCSCTL3 = SELREF__XT2CLK | FLLREFDIV__16; // Set DCO FLL reference = XT2/16

    // Wait for DCO to stabilize again
    do {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);

    UCSCTL5 = DIVA__1 | DIVS__1 | DIVM__1; // Set ACLK, SMCLK, MCLK dividers to 1
    UCSCTL4 = SELA__XT1CLK | SELS__XT2CLK
        | SELM__DCOCLK; // Final clock sources: ACLK=XT1, SMCLK=XT2, MCLK=DCO
}
