#ifndef __LCD_INIT_H
#define __LCD_INIT_H

//#include "sys.h"
#include "main.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 320
#define LCD_H 480

#else
#define LCD_W 480
#define LCD_H 320
#endif

//#define TCS  		PAout(4)  	//PA4  CS2

#define LCD_CS2_Clr()  HAL_GPIO_WritePin(CS2_GPIO_Port,CS2_Pin,0)  	//PA4  CS2
#define LCD_CS2_Set()  HAL_GPIO_WritePin(CS2_GPIO_Port,CS2_Pin,1)  	//PA4  CS2

//#define PEN  		PBin(0)    	//PB0  INT
#define PEN  		HAL_GPIO_ReadPin(PEN_GPIO_Port, PEN_Pin)    	//PB0  INT

//-----------------LCD端口定义---------------- 


#define LCD_RES_Clr()  HAL_GPIO_WritePin(RES_GPIO_Port,RES_Pin,0)//RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(RES_GPIO_Port,RES_Pin,1)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(DC_GPIO_Port,DC_Pin,0)//DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(DC_GPIO_Port,DC_Pin,1)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(BLK_GPIO_Port,BLK_Pin,0)//BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(BLK_GPIO_Port,BLK_Pin,1)

#define LCD_CS_Clr()   HAL_GPIO_WritePin(CS1_GPIO_Port,CS1_Pin,0)//CS1
#define LCD_CS_Set()   HAL_GPIO_WritePin(CS1_GPIO_Port,CS1_Pin,1)




void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif




