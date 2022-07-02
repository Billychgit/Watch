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
//========================================================
void setup() 
{
    cmd_port = &CMD_PORT;
    cmd_port->begin(CMD_PORT_BR);
    READ_EEPROM();
    MainProcess_Init();
    Display_Init();
//  Read DS1307 RTC =====================================
    rtc.begin();
    //rtc.setDate(22, 6, 14);   //設定 RTC　年月日
    //rtc.setTime(9,32, 00);   //設定 RTC　時分秒 
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
     

    pinMode(BUZZ, OUTPUT);
    TimerInit(1, 10000);
    DEBUG_PRINT_DATA_LOG("FW_Version: ", VERSTR);
    //buzzerPlay(500);
}

void loop()
{   
    if(reflash_timer > 1000){
        reflash_timer = 0;
        Display(0,8,1,"  ");
    }
  
       
  
    UserCommand_Task();
    MainProcess_Task();/*%02d:%02d:%02d*/
    //rtc.getDate(runtimedata.ny, runtimedata.nm, runtimedata.nd, runtimedata.weekday);
    //rtc.getTime(runtimedata.nh, runtimedata.nin, runtimedata.second, runtimedata.period);
     //rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    //rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
    //circal();
  
    
    DisplaySetHour();
    DisplaySetMinute();
    DisplaySetYear();
    DisplaySetMonth();
    
    coop();
    
    
    if(runtimedata.UpdateEEPROM)
    {
        runtimedata.UpdateEEPROM = false;
        WRITE_EEPROM();
    }
}
void coop(){
  /*runtimedata.hour=runtimedata.nh;
    runtimedata.minute=runtimedata.nin;
    runtimedata.year=runtimedata.ny;
    runtimedata.month=runtimedata.nm;*/
    rtc.setDate(runtimedata.ny,runtimedata.nm, runtimedata.nd);   //設定 RTC　年月日
    rtc.setTime(runtimedata.nh,runtimedata.nin, 00);   //設定 RTC　時分秒 
  rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
     sprintf(runtimedata.DS1307_DateTime, "%04d/%02d/%02d ", 
        runtimedata.year+2000, runtimedata.month, runtimedata.day);
    cmd_port->println(runtimedata.DS1307_DateTime);
     Display(0,0,0,runtimedata.DS1307_DateTime);
     
     Display(0,0,1,String(runtimedata.hour));
     
     Display(0,2,1,":");
    
     Display(0,4,1,String(runtimedata.minute));
     Display(0,6,1,":");
     
    
     Display(0,8,1,String(runtimedata.second));
    }
void DisplaySetHour()
{
// 
   
  if(digitalRead(22)==HIGH){
    if(runtimedata.hour==23)
    { 
      Display(0,0,1,"  ");
      runtimedata.hour=0;
    }
    else
    { 
      Display(0,0,1,"  ");
      runtimedata.hour++;
    }
  }

  
   if(digitalRead(23)==HIGH)
  {
    if(runtimedata.hour==0)
    {Display(0,0,1,"  ");
     runtimedata.hour=23;
    }
    else
    {
      Display(0,0,1,"  ");
      runtimedata.hour--;
    }
  }
    runtimedata.nh=runtimedata.hour;
  //delay(50);
}



void DisplaySetMinute()
{
// Setting the minutes
  
  if(digitalRead(24)==HIGH)
  {
    if (runtimedata.minute==59)
    { Display(0,4,1,"  ");
      runtimedata.minute=0;
      
    }
    else
    {
      runtimedata.minute++;
      
    }
  }
   if(digitalRead(25)==HIGH)
  {
    if (runtimedata.minute==0)
    { Display(0,4,1,"  ");
      runtimedata.minute=59;
    }
    else
    {
     runtimedata.minute--;
    }
  }
  runtimedata.nin=runtimedata.minute;
  //delay(200);
  
}

void DisplaySetYear()
{
// setting the year
  //lcd.clear();
  if(digitalRead(26)==HIGH)
  {    
    runtimedata.year++;
    runtimedata.ny=runtimedata.year;
    Serial.print("++");
  }
   if(digitalRead(27)==HIGH)
  {
    runtimedata.year--;
    Serial.print("--");
    runtimedata.ny=runtimedata.year;
  }
  
  //delay(200);
}

void DisplaySetMonth()
{
// Setting the month
  
  if(digitalRead(28)==HIGH)
  {
    if (runtimedata.month==12)
    {
      runtimedata.month=1;
      runtimedata.nm=runtimedata.month;
    }
    else
    {
      runtimedata.month=runtimedata.month+1;
      runtimedata.nm=runtimedata.month;
    }
  }
   if(digitalRead(29)==HIGH)
  {
    if (runtimedata.month==1)
    {
      runtimedata.month=12;
      runtimedata.nm=runtimedata.month;
    }
    else
    {
      runtimedata.month=runtimedata.month-1;
      runtimedata.nm=runtimedata.month;
    }
  }
  
  //delay(200);
}

void DisplaySetDay()
{
// Setting the day
  
  if(digitalRead(30)==HIGH)
  {
    if (runtimedata.day==31)
    { 
      runtimedata.day=1;
    }
    else
    {
      runtimedata.day++;
      
    }
  }
   if(digitalRead(31)==HIGH)
  {
    if (runtimedata.day==1)
    {
      runtimedata.day=31;
     
    }
    else
    {
     runtimedata.day--;

    }
  }
 runtimedata.nd=runtimedata.day;
  //delay(200);
}

ISR(TIMER1_COMPA_vect)
{
    if(reflash_timer < 0xFFFF)
        reflash_timer += TIMER_INTERVAL_MS;
        if(test_timer< 0xFFFF)
        test_timer+= TIMER_INTERVAL_MS;
    MainPorcess_Timer();
}
