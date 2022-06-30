#include <Arduino.h>
#include <avr/wdt.h>
#include "SoftwareSerial.h"
#include "UserCommand.h"
#include "EEPROM_Function.h"
#include "MainProcess.h"
#include "hmi.h"
#include "RTCDS1307.h"

#define USER_COMMAND_DEBUG  1
extern RTCDS1307 rtc;
extern HardwareSerial *cmd_port;
extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;
extern DigitalIO digitalio;

CMD g_cmdFunc[] = {
    {"SD", cmd_UpdateEEPROM},
	{"CD", cmd_ClearEEPROM},
	{"RD", cmd_Maindata},
	{"adc", getAdc},
	{"getgpio", getGpio},
	{"setgpio", setGpio},
	{"reset", resetArduino},
	{"getmicros", getMicros},
	{"ver", cmd_CodeVer},
	{"echoon", echoOn},
	{"echooff", echoOff},
	{"out", cmdOutput},
	{"in", cmdInput},
	{"showtime", cmd_ShowNowTime},   
    {"date", cmd_Date},//設定年月日
	{"time", cmd_Time},//設定時分秒
	{"?", showHelp}
};
String g_inputBuffer0 = "";
String* g_inputBuffer = NULL;
String g_cmd = "";
String g_arg = "";

bool g_echoOn = true;

bool getNextArg(String &arg)
{
	if (g_arg.length() == 0)
		return false;
	if (g_arg.indexOf(" ") == -1)
	{
		arg = g_arg;
		g_arg.remove(0);
	}
	else
	{
		arg = g_arg.substring(0, g_arg.indexOf(" "));
		g_arg = g_arg.substring(g_arg.indexOf(" ") + 1);
	}
	return true;
}

void cmd_Maindata(void)
{
	READ_EEPROM();
}
void cmd_UpdateEEPROM(void)
{
	runtimedata.UpdateEEPROM = true;
}
void cmd_ClearEEPROM(void)
{
	Clear_EEPROM();
}
void cmd_Date()
{
	String arg1, arg2, arg3;
	int value;
	bool update = false;
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
	
	if (getNextArg(arg1))
	{
		value = arg1.toInt();
        if (value >= 2000)
        {
            runtimedata.year = value-2000;  
        } 
        else if (value < 100)
        {
            runtimedata.year = value; 
        }    
		update = true;
	}
	if (getNextArg(arg2))
	{
		value = arg2.toInt();
        runtimedata.month = value;
		update = true;
	}
	if (getNextArg(arg3))
	{
		value = arg3.toInt();
        runtimedata.day = value;
		update = true;
	}
	if(update)
	{
        rtc.setDate(runtimedata.year,runtimedata.month,runtimedata.day);    //設定 RTC　年月日
	}
}
void cmd_Time()
{
	String arg1, arg2, arg3;
	int value;
	bool update = false;
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
	
	if (getNextArg(arg1))
	{
		value = arg1.toInt();
        runtimedata.hour = value;
		update = true;
	}
	if (getNextArg(arg2))
	{
		value = arg2.toInt();
        runtimedata.minute = value;
		update = true;
	}
	if (getNextArg(arg3))
	{
		value = arg3.toInt();
        runtimedata.second = value;
		update = true;
	}
	if(update)
	{
        rtc.setTime(runtimedata.hour,runtimedata.minute,runtimedata.second);   //設定 RTC　時分秒 
	}
}

void cmd_ShowNowTime()
{
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);

//    if (!(runtimedata.second % 3)) rtc.setMode(1 - rtc.getMode());
//    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
    
    DEBUG_PRINT(runtimedata.w[runtimedata.weekday - 1]);
    DEBUG_PRINT("  ");
    DEBUG_PRINTDEC(runtimedata.day);
    DEBUG_PRINT("/");
    DEBUG_PRINT(runtimedata.m[runtimedata.month - 1]);
    DEBUG_PRINT("/");
    DEBUG_PRINTDEC(runtimedata.year + 2000);
    DEBUG_PRINT("  ");
    DEBUG_PRINTDEC(runtimedata.hour);
    DEBUG_PRINT(":");
    DEBUG_PRINTDEC(runtimedata.minute);
    DEBUG_PRINT(":");
    DEBUG_PRINTDEC(runtimedata.second);
//    DEBUG_PRINT(rtc.getMode() ? (runtimedata.period ? " PM" : " AM") : "");
    DEBUG_PRINTLN("");
    sprintf(runtimedata.DS1307_DateTime, "%04d/%02d/%02d %02d:%02d:%02d", 
        runtimedata.year+2000, runtimedata.month, runtimedata.day, 
        runtimedata.hour, runtimedata.minute, runtimedata.second);
    cmd_port->println(runtimedata.DS1307_DateTime);
}

void resetArduino(void)
{
	wdt_enable(WDTO_500MS);
	while (1);
}
void getMicros(void)
{
	cmd_port->println(String("micros:") + micros());
}

void showHelp(void)
{
	int i;

	cmd_port->println("");
	for (i = 0; i < (sizeof(g_cmdFunc) / sizeof(CMD)); i++)
	{
		cmd_port->println(g_cmdFunc[i].cmd);
	}
}

void getAdc(void)
{
	String arg1;
	int analogPin;
	int value;

	if (!getNextArg(arg1))
	{
		cmd_port->println("No parameter");
		return;
	}
	analogPin = arg1.toInt();
	value = analogRead(analogPin);
	cmd_port->print("ADC_");
	cmd_port->print(analogPin);
	cmd_port->print(" : ");
	cmd_port->println(value);
}

void getGpio(void)
{
  String arg1, arg2;
  int digitalPin, pullUp;
  int value;

  if (!getNextArg(arg1))
  {
    cmd_port->println("No parameter");
    return;
  }
  if (!getNextArg(arg2))
  {
    pullUp = 0;
  }
  else
  {
    pullUp = arg2.toInt();
  }
  digitalPin = arg1.toInt();
  if (arg2.compareTo("na") == 0)
  {
    cmd_port->println("pin mode keep original");
  }
  else
  {
    if (pullUp)
    {
      cmd_port->println("pull-up");
      pinMode(digitalPin, INPUT_PULLUP);
    }
    else
    {
      cmd_port->println("no-pull");
      pinMode(digitalPin, INPUT);
    }
  }

  cmd_port->print("GPIO:");
  cmd_port->println(arg1);

  value = digitalRead(digitalPin);

  cmd_port->print("input value:");
  cmd_port->println(value);
}

void setGpio(void)
{
  String arg1, arg2;
  int digitalPin;
  int value;

  if (!getNextArg(arg1))
  {
    cmd_port->println("No parameter 1");
    return;
  }
  if (!getNextArg(arg2))
  {
    cmd_port->println("No parameter 2");
    return;
  }
  digitalPin = arg1.toInt();
  value = arg2.toInt();

  cmd_port->print("GPIO:");
  cmd_port->println(arg1);
  cmd_port->print("level:");
  cmd_port->println(arg2);

  digitalWrite(digitalPin, value ? HIGH : LOW);
  pinMode(digitalPin, OUTPUT);
}

void echoOn(void)
{
  g_echoOn = true;
}

void echoOff(void)
{
  g_echoOn = false;
}

void cmd_CodeVer(void)
{
  cmd_port->println(VERSTR);
}

void cmdOutput(void)
{
	String arg1, arg2;
	int digitalPin;
	int value;

	if (!getNextArg(arg1))
	{
		cmd_port->println("No parameter 1");
		return;
	}
	if (!getNextArg(arg2))
	{
		cmd_port->println("No parameter 2");
		return;
	}
	digitalPin = arg1.toInt();
	value = arg2.toInt();

	cmd_port->print("PIN index:");
	cmd_port->println(arg1);
	cmd_port->print("level:");
	cmd_port->println(arg2);

	setOutput((uint8_t)digitalPin, (uint8_t)(value ? HIGH : LOW));
	cmd_port->println("");
}

void cmdInput(void)
{
	String arg1;
	unsigned long pinindex;

	getNextArg(arg1);
	if( (arg1.length()==0))
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	pinindex = arg1.toInt();
	cmd_port->println("Sensor: " + String(getInput(pinindex)));
}


uint8_t UserCommWorkindex = 0;

uint32_t UserCommandTimeCnt = 0;

void UserCommand_Task(void)
{
  int i, incomingBytes, ret, cmdPortIndex;
  char data[2] = {0};

  switch(UserCommWorkindex)
  {
    case 0:
    {
      
      if(cmd_port->available())
      {
        g_inputBuffer = &g_inputBuffer0;
        UserCommWorkindex ++;
        UserCommandTimeCnt = millis();
      }
      break;
    }
    case 1:
    {
      if((millis() - UserCommandTimeCnt) > 50)
        UserCommWorkindex ++;
      break;
    }
    case 2:
    {
      if ( incomingBytes = cmd_port->available() )
      {

//      cmd_port->println("cmd_port datalen: " + String(incomingBytes));
//      Serial3.println("cmd_port datalen: " + String(incomingBytes));
      for ( i = 0; i < incomingBytes; i++ )
      {
        ret = cmd_port->read();
        if ( (ret >= 0x20) && (ret <= 0x7E) || (ret == 0x0D) || (ret == 0x0A) )
        {
        data[0] = (char)ret;
        (*g_inputBuffer) += data;
        if (g_echoOn)
        {
          if ( (data[0] != 0x0D) && (data[0] != 0x0A) )
          cmd_port->write(data);
        }
        }
        else if (ret == 0x08)
        {
        if (g_inputBuffer->length())
        {
          g_inputBuffer->remove(g_inputBuffer->length() - 1);
          if (g_echoOn)
          {
          data[0] = 0x08;
//          cmd_port->write(data);
          }
        }
        }
      }
      if (g_inputBuffer->indexOf('\r') == -1)
      {
        if (g_inputBuffer->indexOf('\n') == -1)
        return;
      }
      g_inputBuffer->trim();
      while (g_inputBuffer->indexOf('\r') != -1)
        g_inputBuffer->remove(g_inputBuffer->indexOf('\r'), 1);
      while (g_inputBuffer->indexOf('\n') != -1)
        g_inputBuffer->remove(g_inputBuffer->indexOf('\n'), 1);
      while (g_inputBuffer->indexOf("  ") != -1)
        g_inputBuffer->remove(g_inputBuffer->indexOf("  "), 1);
    
      cmd_port->println();
    
      if (g_inputBuffer->length())
      {
        g_arg.remove(0);
        if (g_inputBuffer->indexOf(" ") == -1)
        g_cmd = (*g_inputBuffer);
        else
        {
        g_cmd = g_inputBuffer->substring(0, g_inputBuffer->indexOf(" "));
        g_arg = g_inputBuffer->substring(g_inputBuffer->indexOf(" ") + 1);
        }
        for (i = 0; i < (sizeof(g_cmdFunc) / sizeof(CMD)); i++)
        {
        //if(g_cmd==g_cmdFunc[i].cmd)
        if (g_cmd.equalsIgnoreCase(g_cmdFunc[i].cmd))
        {
          g_cmdFunc[i].func();
//          cmd_port->print("ARDUINO>");
          break;
        }
        else if (i == (sizeof(g_cmdFunc) / sizeof(CMD) - 1))
        {
//          cmd_port->println("bad command !!");
//          cmd_port->print("ARDUINO>");
        }
        }
        *g_inputBuffer = "";
      }
      else
      {
//        cmd_port->print("ARDUINO>");
      }
      UserCommWorkindex = 0;
      break;
    }
  }

  }
}
