#ifndef _HMI_CMD_H_
#define _HMI_CMD_H_

#include "Arduino.h"

#define	VERSTR	"2022060701"
#define VENDOR	"LH_RTC"
#define CHECH_VERSION 0x060701

#define NeenDebugLog
//#undef NeenDebugLog

#ifdef NeenDebugLog
    #define DEBUG_PRINT_VERBOSE(str, value) \
                    Serial.print(F(str)); \
                    Serial.println(value); \
                    Serial.print(F("[LINE]: ")); \
                    Serial.println(__LINE__); \
                    Serial.print(F("[FUNC]: ")); \
                    Serial.println(__func__);
    
    #define DEBUG_PRINT_DATA_LOG(str, value) \
                    Serial.print(F(str)); \
                    Serial.println(value);
    #define DEBUG_PRINT(str) Serial.print((str))
    #define DEBUG_PRINTLN(str) Serial.println((str))
    #define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
    #define DEBUG_PRINTHEX(x) Serial.print (x, HEX)
#else
    #define DEBUG_PRINT_VERBOSE(str,value)
    #define DEBUG_PRINT_DATA_LOG(str,value)
    #define DEBUG_PRINT(str)
    #define DEBUG_PRINTLN(str)
    #define DEBUG_PRINTDEC(x)
    #define DEBUG_PRINTHEX(x)
#endif

#define setbit(value,x) (value |=(1<<x))
#define getbit(value,x) ((value>>x)&1)
#define clrbit(value,x) (value &=~(1<<x))
#define revbit(value,x) (value ^=(1<<x))

#define RST_PIN      42        // RFID_RST 腳位 
#define MISO         50        // MISO 腳位
#define M0SO         51        // MOSO 腳位
#define SCK          52        // SCK  腳位
#define SS_PIN       53        // SS   腳位

#define IN0          22
#define IN1          23
#define IN2          24
#define IN3          25
#define IN4          26
#define IN5          27
#define IN6          28
#define IN7          29

#define IN8          30
#define IN9          31
#define IN10         32
#define IN11         33
#define IN12         34
#define IN13         35
#define IN14         36
#define IN15         37


#define FLASH_CS     47        // FLASH ROM CS PIN
#define BUZZ         48 
#define BT_PWRC      49        // BT4.2 

#define PWM_2         2        // MPU 直接產生
#define PWM_3         3        // MPU 直接產生
#define PWM_6         6        // MPU 直接產生
#define PWM_7         7        // MPU 直接產生
#define PWM_8         8        // MPU 直接產生
#define PWM_11       11        // MPU 直接產生
#define PWM_12       12        // MPU 直接產生
#define PWM_44       44        // MPU 直接產生
#define PWM_45       45        // MPU 直接產生

#define	OUTPUT_TOTAL	8
#define	INPUT_TOTAL		16

static const uint8_t OutputPin[] = {A0, A1, A2, A3, A4, A5, A6, A7};

static const uint8_t InputPin[] = {22, 23, 24, 25, 26, 27, 28, 29,
								   30, 31, 32, 33, 34, 35, 36, 37};

static const uint8_t ADC_PWMPin[] = {0, 0, 0, A8, 2, A9, 3, A10, 6, A11, 7, A12,
									11, A13, 12, A14, 44, A15, 45, 0, 0};


#define	CMD_PORT		Serial	
#define	CMD_PORT_BR		115200

#endif
