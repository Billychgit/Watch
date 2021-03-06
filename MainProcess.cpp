#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "hmi.h"
#include "Timer.h"
#include "RTCDS1307.h"
#include "EEPROM_Function.h"

extern RTCDS1307 rtc;
extern HardwareSerial *cmd_port;
MainDataStruct maindata;
RuntimeStatus runtimedata;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];

void MainPorcess_Timer()
{
}
void MainProcess_ReCheckEEPROMValue()
{
    uint8_t i = 0;
    if(maindata.CheckVersion != CHECH_VERSION){
        maindata.CheckVersion = CHECH_VERSION;
    }
    runtimedata.UpdateEEPROM = true;
}

void MainProcess_Init()
{
    int i, j;
    runtimedata.UpdateEEPROM = false;
    MainProcess_ReCheckEEPROMValue();

    for (i = 0; i < WORKINDEX_TOTAL; i++)
        runtimedata.Workindex[i] = 0x00;
    for (i = 0; i < WORKINDEX_TOTAL; i++)
        runtimedata.preWorkindex[i] = -1;

    for (i = 0; i < INPUT_8_NUMBER + EXTIO_NUM; i++)
        digitalio.Input[i] = 0;

    for (i = 0; i < OUTPUT_8_NUMBER + EXTIO_NUM; i++)
    {
        if (OUTPUT_NONE_ACTIVE == 0)
            digitalio.Output[i] = 0;
        else
            digitalio.Output[i] = 0xFF;
    }

    for (i = 0; i < INPUT_8_NUMBER * 8; i++)
    {
        pinMode(InputPin[i], INPUT);
    }
    for (i = 0; i < OUTPUT_8_NUMBER * 8; i++)
    {
        pinMode(OutputPin[i], OUTPUT);
    }

    for (j = 0; j < EXTIO_NUM; j++)
    {
        extio[j].begin(j); // Default device address 0x20+j

        for (i = 0; i < 8; i++)
        {
            extio[j].pinMode(i, OUTPUT); // Toggle LED 1
            extio[j].digitalWrite(i, OUTPUT_NONE_ACTIVE);
        }
    }
    for (i = 0; i < OUTPUT_8_NUMBER * 8; i++)
        digitalWrite(OutputPin[i], OUTPUT_NONE_ACTIVE);

    for (j = 0; j < EXTIO_NUM; j++)
        for (i = 0; i < 8; i++)
        {
            extio[j].pinMode(i + 8, INPUT); // Button i/p to GND
            extio[j].pullUp(i + 8, HIGH);   // Puled high to ~100k
        }
    runtimedata.RunMode = RUN_MODE_FREE;
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

void MainProcess_Task()  
{
    if(runtimedata.preRunMode != runtimedata.RunMode)
    {
        runtimedata.preRunMode = runtimedata.RunMode;
        cmd_port->println("RunMode: " + String(runtimedata.preRunMode));
    }
	switch(runtimedata.RunMode)
	{
        case RUN_MODE_FREE:
            break;
	}
}

void buzzerPlay(int playMS)
{
  digitalWrite(BUZZ, HIGH);
  delay(playMS);
  digitalWrite(BUZZ, LOW);
}
