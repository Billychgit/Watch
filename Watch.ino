#include <Arduino.h>
#include "UserCommand.h"
#include "EEPROM_Function.h"
#include <EEPROM.h>
#include "MainProcess.h"
#include "hmi.h"
#include "Timer.h"
#include "Display.h"
#include "RTCDS1307.h"
#include <Adafruit_MCP23017.h>
#include <Wire.h>

HardwareSerial *cmd_port;

extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;

void setup() {
  cmd_port = &CMD_PORT;
  cmd_port->begin(CMD_PORT_BR);
  READ_EEPROM();
  MainProcess_Init();
  TimerInit(1, 10000);
  }

void loop() {
  UserCommand_Task();
  MainProcess_Task();
   if(runtimedata.UpdateEEPROM)
 {
    runtimedata.UpdateEEPROM = false;
    WRITE_EEPROM(); //maindata內的值都會寫到EEPROM
  }}
  ISR(TIMER1_COMPA_vect)
{
    MainProcessTimer();
}
