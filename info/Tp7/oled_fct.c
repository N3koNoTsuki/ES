#include "oled.h"

void clearcmd(uint8_t* cmd, size_t len){
    memset(cmd, 0, len);
}

int CleaOled(int __Fd){
    uint8_t cmd[2] = {0xFF, 0xD7};
    uint8_t RetVal;
   	#ifdef DEBUG
    fprintf(stdout, "HI Co\n\r");
    #endif
	if (write(__Fd, cmd, sizeof(cmd)) < 0)
	{
		return -1;
	}
   	#ifdef DEBUG
    fprintf(stdout, "HI2 Co\n\r");
    #endif
	if (read(__Fd, &RetVal, sizeof(RetVal)) < 0)
	{
		return -2;
	}

    return RetVal;
}

int SendCmd(int __Fd, const uint8_t * __buf, size_t len){

    int RetVal;

    #ifdef DEBUG
    fprintf(stdout, "HI\n\r");
    #endif

	ssize_t written = write(__Fd, __buf, len);
	if (written < 0)
	{
		return -1;
	}

    #ifdef DEBUG
    perror("write ");
    fprintf(stdout, "HI2\n\r");
    #endif

	ssize_t read_len = read(__Fd, &RetVal, sizeof(RetVal));
	if (read_len < 0)
	{
		return -2;
	}

    #ifdef DEBUG
    perror("read ");
    #endif
    
    return RetVal;
}
