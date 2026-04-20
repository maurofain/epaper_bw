#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

//#define  Arduino_UNO_R4
#define  ESP8266

#ifdef ESP8266
//IO settings
#define isEPD_W21_BUSY digitalRead(D0)  //BUSY
#define EPD_W21_RST_0 digitalWrite(D1,LOW) //RES
#define EPD_W21_RST_1 digitalWrite(D1,HIGH)
#define EPD_W21_CS2_0  digitalWrite(D2,LOW)  //CS2
#define EPD_W21_CS2_1  digitalWrite(D2,HIGH)
#define EPD_W21_CS_0 digitalWrite(D4,LOW) //CS
#define EPD_W21_CS_1 digitalWrite(D4,HIGH)

#define EPD_W21_CLK_0 digitalWrite(D5,LOW)
#define EPD_W21_CLK_1 digitalWrite(D5,HIGH)
#define EPD_W21_MOSI_0  digitalWrite(D7,LOW)
#define EPD_W21_MOSI_1  digitalWrite(D7,HIGH) 

void Sys_run(void);
void LED_run(void);    
#endif 

#ifdef Arduino_UNO_R4
//IO settings
//HSCLK---13
//HMOSI--11
#define isEPD_W21_BUSY digitalRead(4) 
#define EPD_W21_RST_0 digitalWrite(5,LOW)
#define EPD_W21_RST_1 digitalWrite(5,HIGH)
#define EPD_W21_CS2_0  digitalWrite(6,LOW)
#define EPD_W21_CS2_1  digitalWrite(6,HIGH)
#define EPD_W21_CS_0 digitalWrite(7,LOW)
#define EPD_W21_CS_1 digitalWrite(7,HIGH)

#define EPD_W21_CLK_0 digitalWrite(13,LOW)
#define EPD_W21_CLK_1 digitalWrite(13,HIGH)
#define EPD_W21_MOSI_0  digitalWrite(11,LOW)
#define EPD_W21_MOSI_1  digitalWrite(11,HIGH) 
#endif 


void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);
void EPD_W21_WriteDATA1(unsigned char datas);
void EPD_W21_WriteCMD1(unsigned char command);
void EPD_W21_WriteDATA2(unsigned char datas);
void EPD_W21_WriteCMD2(unsigned char command);

#endif 
