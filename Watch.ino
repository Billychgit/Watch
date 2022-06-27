  #include <Arduino.h>
#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "hmi.h"
#include "Timer.h"
#include <EEPROM.h>
#include "RTCDS1307.h"
#include "EEPROM_Function.h"
#include "UserCommand.h"
#include "Display.h"
#include <Wire.h>


HardwareSerial *cmd_port;

extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;

RTCDS1307 rtc(0x68);

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}
double map(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
long reflash_timer = 0;
long test_timer=0;

void setup() {
  cmd_port = &CMD_PORT;
  cmd_port->begin(CMD_PORT_BR);
  READ_EEPROM();
  MainProcess_Init();
  TimerInit(1, 10000);
  Display_Init();
  
 
 rtc.begin();
 rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
 rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
  
  }

void loop() {
  
  /*if(reflash_timer > 1000){
        reflash_timer = 0;
        Display(0,8,1,"  ");*/
  UserCommand_Task();
  MainProcess_Task();
  

    
   if(runtimedata.UpdateEEPROM)
 {
    runtimedata.UpdateEEPROM = false;
    WRITE_EEPROM(); //maindata內的值都會寫到EEPROM
  }
  
  }
  ISR(TIMER1_COMPA_vect)
{
    MainProcessTimer();
 if(reflash_timer < 0xFFFF)
        reflash_timer += TIMER_INTERVAL_MS;
}
