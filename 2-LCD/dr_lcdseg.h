/*
 * dr_lcdseg.h
 *
 *  Created on: 2013-5-28
 *      Author: DengPihui
 */

#ifndef DR_LCDSEG_H_
#define DR_LCDSEG_H_

#define CHAR_S 17
#define CHAR_J 18
#define CHAR_T 19
#define CHAR_U 20

void initLcdSeg();	//显示频初始化
void LCDSEG_SetDigit(int pos, int value); //在pos(0<=pos<=5)位置写入一个整数value（-1《=value《=16，0《=pos《=5），value=16表示写入负号，value=-1表示清除该位
void LCDSEG_SetSpecSymbol(int pos);//在pos位置清除小数点（3<=pos<=5）
void LCDSEG_ResetSpecSymbol(int pos);//在pos位置写小数点（3<=pos<=5）
void LCDSEG_DisplayNumber(int32_t num, int dppos);//让段式液晶显示num这个整数，并在dppos位置处添加小数点(《0《dppos《=3)
void LCDSEG_DisplayNumString(char * num);


#endif /* DR_LCDSEG_H_ */
