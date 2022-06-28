#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "hmi.h"
#include "Timer.h"
#include "Display.h"
#include "RTCDS1307.h"
#include "EEPROM_Function.h"
#include <RTClib.h>


extern RTCDS1307 rtc;
extern HardwareSerial *cmd_port;
MainDataStruct maindata;
RuntimeStatus runtimedata;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];





int hourupg;
int minupg;
int yearupg;
int monthupg;
int dayupg;
int menu =0;
int P2=23;
int P3=24;
void MainProcess_ReCheckEEPROMValue()
{
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
		runtimedata.UpdateEEPROM = true;
	}
    if(maindata.SettingTimer < 0)
    {
        maindata.SettingTimer = 1000;
		runtimedata.UpdateEEPROM = true;
    }
}
void MainProcessTimer()
{
  
    //if(WaitTimer < 0xFF00)
        //WaitTimer += TIMER_INTERVAL_MS;
    //if(FreeTimer < 0xFF00)
        //FreeTimer += TIMER_INTERVAL_MS;
    
        
}
void MainProcess_Init()
{
	int i,j;
	runtimedata.UpdateEEPROM = false;
	MainProcess_ReCheckEEPROMValue();

	for(i=0;i<INPUT_8_NUMBER+EXTIO_NUM;i++)
		digitalio.Input[i] = 0;
	
	for(i=0;i<OUTPUT_8_NUMBER+EXTIO_NUM;i++)
	{
		if(OUTPUT_NONE_ACTIVE == 0)
			digitalio.Output[i]	= 0;
		else
			digitalio.Output[i]	= 0xFF;
	}
		
	for(i=0; i<INPUT_8_NUMBER*8; i++)
	{
		pinMode(InputPin[i], INPUT);
	}
	for(i=0; i<OUTPUT_8_NUMBER*8; i++)
	{
		pinMode(OutputPin[i], OUTPUT);	
	}
	
	for(j=0; j<EXTIO_NUM; j++)
	{
		extio[j].begin(j);	  	// Default device address 0x20+j

		for(i=0; i<8; i++)
		{
			extio[j].pinMode(i, OUTPUT);  // Toggle LED 1
			extio[j].digitalWrite(i, OUTPUT_NONE_ACTIVE);
		}
	}
	for(i=0; i<OUTPUT_8_NUMBER*8; i++)
		digitalWrite(OutputPin[i], OUTPUT_NONE_ACTIVE);

	for(j=0; j<EXTIO_NUM; j++)
		for(i=0; i<8; i++)
		{
			extio[j].pinMode(i+8,INPUT);	 // Button i/p to GND
			extio[j].pullUp(i+8,HIGH);	 // Puled high to ~100k
		}
    runtimedata.Workindex[0] = 2;
}

void ReadDigitalInput()
{
	uint8_t i,bi, j, value;
	String outstr = "";
	bool inputupdate = false;
	uint8_t input8 = 1;
	
	for(i=0; i<8; i++)
	{
		runtimedata.sensor[i] = digitalRead(InputPin[i]);
		if(runtimedata.sensor[i])
			{setbit(digitalio.Input[0], i);	}
		else
			{clrbit(digitalio.Input[0], i);	}
	}

	if(INPUT_8_NUMBER == 2)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+8] = digitalRead(InputPin[i+8]);
			
			if(runtimedata.sensor[i+8])
				{setbit(digitalio.Input[1], i); }
			else
				{clrbit(digitalio.Input[1], i); }
		}
		input8 += 1;
	}

	if(EXTIO_NUM > 0)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+8] = extio[0].digitalRead(i+8);
				
			if(runtimedata.sensor[i+input8*8])
				{setbit(digitalio.Input[input8], i);	}
			else
				{clrbit(digitalio.Input[input8], i);	}
		}
		input8 += 1;
	}
	if(EXTIO_NUM > 1)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+input8*8] = extio[1].digitalRead(i+8);
			if(runtimedata.sensor[i+input8*8])
				{setbit(digitalio.Input[input8], i);	}
			else
				{clrbit(digitalio.Input[input8], i);	}
		}
	}

}

void WriteDigitalOutput()
{
	uint8_t i,bi, j, value;

	for(i=0; i<OUTPUT_8_NUMBER+EXTIO_NUM; i++)
	{
		if(digitalio.PreOutput[i] != digitalio.Output[i])
		{
			digitalio.PreOutput[i] = digitalio.Output[i];
			
			switch(i)
			{
				case 0: //onboard
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						digitalWrite(OutputPin[bi], value);
					}
					break;

				case 1: //extern board 0
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						if(OUTPUT_8_NUMBER == 2)
							digitalWrite(OutputPin[bi+8], value);
						else
							extio[0].digitalWrite(bi, value);
					}
					break;
				case 2: //extern board 1
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						if(OUTPUT_8_NUMBER == 2)
							extio[0].digitalWrite(bi, value);
						else
							extio[1].digitalWrite(bi, value);
					}
					break;
				case 3: //extern board 1
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						extio[1].digitalWrite(bi, value);
					}
					break;
			}	
		}
	}
}

void setOutput(uint8_t index, uint8_t hl)
{
	if(index < (OUTPUT_8_NUMBER*8))
	{
		digitalWrite(OutputPin[index], hl);
	}
	else
	{
		uint8_t extindex = index-(OUTPUT_8_NUMBER*8);
		uint8_t exi = extindex >> 3;
		uint8_t bi = extindex & 0x07;
		extio[exi].digitalWrite(bi, hl);
	}
	digitalio.Output[index] = hl;
}

uint8_t getInput(uint8_t index)
{
	uint8_t hl;
	if(index < (INPUT_8_NUMBER*8))
	{
		hl = digitalRead(InputPin[index]);
	}
	else
	{
		uint8_t extindex = index-(INPUT_8_NUMBER*8);
		uint8_t exi = extindex >> 3;
		uint8_t bi = extindex & 0x07;
		hl = extio[exi].digitalRead(bi+8);
	}

	digitalio.Input[index] = hl;
	return hl;
}

int n=1;
int i=1; int o=0;

void MainProcess_Task()  // This is a task.
{
    if( digitalRead(22)){ menu=menu+1;}
    
    switch(menu)
    {
        case 0 :
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
          
            break;
            
        
        case 1 : 
            DisplaySetYear();
            break;
            
        case 2:
          DisplaySetMonth();
          break;

          case 3:
           DisplaySetDay();
          break;

           case 4:
           DisplaySetHour();
          break;

           case 5:
           DisplaySetMinute();
          break;

          case 6 :
          menu =0;
          break;
      }
}

    
void DisplaySetHour()
{
// time setting
DateTime now = rtc.now();
   
  if(digitalRead(P2)==HIGH){
    if(runtimedata.hour==23)
    {
      runtimedata.hour=0;
    }
    else
    {
      runtimedata.hour=runtimedata.hour+1;
    }
  }

  
   if(digitalRead(P3)==HIGH)
  {
    if(runtimedata.hour==0)
    {
     runtimedata.hour=23;
    }
    else
    {
      runtimedata.hour=runtimedata.hour-1;
    }
  }
  
  delay(200);
}

void DisplaySetMinute()
{
// Setting the minutes
  
  if(digitalRead(P2)==HIGH)
  {
    if (runtimedata.minute==59)
    {
      runtimedata.minute=0;
    }
    else
    {
      runtimedata.minute=runtimedata.minute+1;
    }
  }
   if(digitalRead(P3)==HIGH)
  {
    if (runtimedata.minute==0)
    {
      runtimedata.minute=59;
    }
    else
    {
      runtimedata.minute=runtimedata.minute-1;
    }
  }
  
  delay(200);
}
  
void DisplaySetYear()
{
// setting the year
  //lcd.clear();
  if(digitalRead(P2)==HIGH)
  {    
    runtimedata.year=runtimedata.year+1;
  }
   if(digitalRead(P3)==HIGH)
  {
    runtimedata.year=runtimedata.year-1;
  }
  
  delay(200);
}

void DisplaySetMonth()
{
// Setting the month
  
  if(digitalRead(P2)==HIGH)
  {
    if (runtimedata.month==12)
    {
      runtimedata.month=1;
    }
    else
    {
      runtimedata.month=runtimedata.month+1;
    }
  }
   if(digitalRead(P3)==HIGH)
  {
    if (runtimedata.month==1)
    {
      runtimedata.month=12;
    }
    else
    {
      runtimedata.month=runtimedata.month-1;
    }
  }
  
  delay(200);
}

void DisplaySetDay()
{
// Setting the day
  //lcd.clear();
  if(digitalRead(P2)==HIGH)
  {
    if (runtimedata.day==31)
    {
      runtimedata.day=1;
    }
    else
    {
      runtimedata.day=runtimedata.day+1;
    }
  }
   if(digitalRead(P3)==HIGH)
  {
    if (runtimedata.day==1)
    {
      runtimedata.day=31;
    }
    else
    {
      runtimedata.day=runtimedata.day-1;
    }
  }
 
  delay(200);
}




    
  
  
  

void buzzerPlay(int playMS)
{
  digitalWrite(BUZZ, HIGH);
  delay(playMS);
  digitalWrite(BUZZ, LOW);
}
