#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"

#define EXTIO_NUM 0
#define INPUT_8_NUMBER 2
#define OUTPUT_8_NUMBER 1

#define OUTPUT_NONE_ACTIVE 0
#define OUTPUT_ACTIVE 1

#define INPUT_NONE_ACTIVE 0
#define INPUT_ACTIVE 1

#define RUN_MODE_EMERGENCY		        -1
#define RUN_MODE_STOP		            0
#define RUN_MODE_GO_HOME                1
#define RUN_MODE_INIT                   2
#define RUN_MODE_FREE                   3



#define WORKINDEX_TOTAL		        5

#define WORKINDEX_GO_HOME               0
#define WORKINDEX_INIT                  1

typedef struct _DigitalIO_
{
	uint8_t	Input[4];
	uint8_t	Output[4];
	uint8_t PreOutput[4];
}DigitalIO;

typedef struct _MainDataStruct_
{
	char   Vendor[10];
    uint32_t CheckVersion;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];
	
	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];

    int RunMode = 0;
    int preRunMode = -1;
    
	bool 		UpdateEEPROM;
    //RTC DS1307 =========================================================
    uint8_t year, month, day, hour, minute,second,weekday,ny,nm,nd,nh,nin;
    bool period = 0;
    String m[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    String w[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char DS1307_DateTime[25];
}RuntimeStatus;

void setOutput(uint8_t index, uint8_t hl);
uint8_t getInput(uint8_t index);

void MainPorcess_Timer();
void MainProcess_Task();
void MainProcess_Init();
void buzzerPlay(int playMS);

#endif	//_MAIN_PROCESS_H_
