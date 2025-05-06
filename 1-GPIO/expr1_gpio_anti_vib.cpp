/*
 * main.c
 */
#include <msp430.h>

#include <stdint.h>
typedef struct                   //以指针形式定义P8口的各个寄存器
{
  const volatile uint8_t* PxIN;     //定义一个不会被编译的无符号字符型变量
  volatile uint8_t* PxOUT;
  volatile uint8_t* PxDIR;
  volatile uint8_t* PxREN;
  volatile uint8_t* PxSEL;
} GPIO_TypeDef;

const GPIO_TypeDef GPIO4 =
{ &P4IN, &P4OUT, &P4DIR, &P4REN, &P4SEL};

const GPIO_TypeDef GPIO5 =
{&P5IN, &P5OUT, &P5DIR, &P5REN, &P5SEL};

const GPIO_TypeDef GPIO8 =
{&P8IN, &P8OUT, &P8DIR, &P8REN, &P8SEL};

const GPIO_TypeDef* LED_GPIO[5] = {&GPIO4, &GPIO4, &GPIO4, &GPIO5, &GPIO8};
const uint8_t LED_PORT[5] = {BIT5, BIT6, BIT7, BIT7, BIT0};

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  while(BAKCTL & LOCKIO) // Unlock XT1 pins for operation
    BAKCTL &= ~(LOCKIO);
  UCSCTL6 &= ~XT1OFF; //启动XT1
  while (UCSCTL7 & XT1LFOFFG) //等待XT1稳定
    UCSCTL7 &= ~(XT1LFOFFG);

  UCSCTL4 = SELA__XT1CLK + SELS__REFOCLK + SELM__REFOCLK; //时钟设为XT1，频率较低，方便软件延时

  int i;
  for(i=0;i<5;++i)
    *LED_GPIO[i]->PxDIR |= LED_PORT[i]; //设置各LED灯所在端口为输出方向

  P4REN |= 0x1F; //使能按键端口上的上下拉电阻
  P4OUT |= 0x1F; //上拉状态

  uint8_t btn_status, last_btn_status = 0xFF;
  uint8_t led_count = 0;  // 当前点亮的LED数量
  uint8_t debounce_count = 0;  // 防抖计数器
  const uint8_t DEBOUNCE_THRESHOLD = 2;  // 防抖阈值
  
  while(1)
  {
    btn_status = P4IN & 0x1F;  // 读取按键状态
    
    if(btn_status == last_btn_status)
    {
      if(debounce_count < DEBOUNCE_THRESHOLD)
      {
        debounce_count++;
      }
      else if(debounce_count == DEBOUNCE_THRESHOLD)
      {
        if((btn_status & 0x08) == 0)  // 增加LED按键
        {
          if(led_count < 5)
          {
            led_count++;
            int i;
            for(i = 0; i < 5; i++)
            {
              if(i < led_count)
                *LED_GPIO[i]->PxOUT |= LED_PORT[i];
              else
                *LED_GPIO[i]->PxOUT &= ~LED_PORT[i];
            }
          }
        }
        else if((btn_status & 0x10) == 0)  // 清零按键
        {
          led_count = 0;
          int i;
          for(i = 0; i < 5; i++)
          {
            *LED_GPIO[i]->PxOUT &= ~LED_PORT[i];
          }
        }
        debounce_count++;  // 防止重复触发
      }
    }
    else
    {
      debounce_count = 0;
      last_btn_status = btn_status;
    }
    
    __delay_cycles(100);  // 缩短延时间隔，提高采样频率
  }
}
