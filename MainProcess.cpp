#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "hmi.h"
#include "Timer.h"
#include "Display.h"


extern HardwareSerial *cmd_port;
MainDataStruct maindata;
RuntimeStatus runtimedata;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];

uint16_t Out_Timer = 0;
uint16_t WaitTimer = 0;
uint16_t FreeTimer = 0;
uint16_t AddPressTimer = 0;
uint16_t SubPressTimer = 0;

uint16_t AddSubDelayTimer = 0;
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
    if(Out_Timer < 0xFFFF)
        Out_Timer += TIMER_INTERVAL_MS;
    if(WaitTimer < 0xFF00)
        WaitTimer += TIMER_INTERVAL_MS;
    if(FreeTimer < 0xFF00)
        FreeTimer += TIMER_INTERVAL_MS;
    
        
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
    switch(runtimedata.RunMode)
    {
        case 0 :
            
              if(digitalRead(22)==HIGH){
            //觸發進入調整模式
            
            Display(0,0,0,"                ");
              }
            break;
            runtimedata.RunMode=1;
        
        case 1 :

            //選擇不同的項目
            int count=1;
            if(digitalRead(23)==HIGH)
            {
              count++;
              if(count==5)count=5;
            }

            Serial.println(count);
            
            break;
            
        
        case 2:

         // 加減
            
            break;

        case 3:

          //結束設置
          break;
      }
}
int AddTimesMode = 0;
int SubTimesMode = 0;
int preAddTimesMode = -1;
int preSubTimesMode = -1;
uint8_t preAddBtnState_times = 0;
uint8_t preSubBtnState_times = 0;
void SetTimesProcess()
{    
    uint8_t addbtn = getInput(24);
    uint8_t subbtn = getInput(25);

    if((addbtn && !preAddBtnState_times)){
        FreeTimer = 0;

        
            if(preAddTimesMode != AddTimesMode)
            {
                preAddTimesMode = AddTimesMode;
                cmd_port->println("AddTimesMode: " + String(preAddTimesMode));
            }
            switch(AddTimesMode)
            {
                case 0:
                    if(AddPressTimer >= 2000)
                    {
                        AddTimesMode += 10;
                    }
                    if(AddSubDelayTimer > 500){ //debug時可修改
                        maindata.TotalTimes += 1;
                        AddSubDelayTimer = 0;
                    }
                    if(maindata.TotalTimes >= MAX_TOTAL_TIMES) maindata.TotalTimes = MAX_TOTAL_TIMES;
                    Display(0, 0, 1, "Total times: " + String(maindata.TotalTimes) + "          ");
                    break;
                case 10:
                    maindata.TotalTimes += 1;
                    if(maindata.TotalTimes >= MAX_TOTAL_TIMES) maindata.TotalTimes = MAX_TOTAL_TIMES;
                    Display(0, 0, 1, "Total times: " + String(maindata.TotalTimes) + "          ");
                    break;
            }
        }
    
    if((subbtn && !preSubBtnState_times)){
        FreeTimer = 0;

        
            if(preSubTimesMode != SubTimesMode)
            {
                preSubTimesMode = SubTimesMode;
                cmd_port->println("SubTimesMode: " + String(preSubTimesMode));
            }
            switch(SubTimesMode)
            {
                case 0:
                    if(SubPressTimer >= 2000)
                    {
                        SubTimesMode += 10;
                    }
                    if(AddSubDelayTimer > 500){ //debug時可修改
                        maindata.TotalTimes -= 1;
                        AddSubDelayTimer = 0;
                    }
                    if(maindata.TotalTimes <= MIN_TOTAL_TIMES) maindata.TotalTimes = MIN_TOTAL_TIMES;
                    Display(0, 0, 1, "Total times: " + String(maindata.TotalTimes) + "           ");
                    break;
                case 10:
                    maindata.TotalTimes -= 1;
                    if(maindata.TotalTimes <= MIN_TOTAL_TIMES) maindata.TotalTimes = MIN_TOTAL_TIMES;
                    Display(0, 0, 1, "Total times: " + String(maindata.TotalTimes) + "           ");
                    break;
            }
        }
    }
int ele=0;
void changeProcess()
{
  switch (ele)
  {
    
  case 0 :

  sprintf(runtimedata.DS1307_DateTime, "%04d", 
        runtimedata.year);

   break;

    case 1:

    Serial.println("b");
ele=3;
    break;

 case 2:

    Serial.println("c");
ele=1;
    break;

     case 3:

    Serial.println("d");

    break;
    
  }
  }
  
  
  

void buzzerPlay(int playMS)
{
  digitalWrite(BUZZ, HIGH);
  delay(playMS);
  digitalWrite(BUZZ, LOW);
}
