#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;

void cmd_Maindata(void);
void cmd_UpdateEEPROM(void);
void cmd_ClearEEPROM(void);
void resetArduino(void);
void getMicros(void);
void getAdc(void);
void getGpio(void);
void setGpio(void);
void echoOn(void);
void echoOff(void);
void cmd_CodeVer(void);
void showHelp(void);
bool getNextArg(String &arg);
void cmdOutput(void);
void cmdInput(void);
void cmd_Date();
void cmd_Time();
void cmd_ShowNowTime();

void UserCommand_Task(void);
void UserCommand_Timer(void);
#endif //_USER_COMMAND_H_
