/* $Id: device.h, v 1.0 05/07/2017 11:51:52  $
 *
 * Elinux : 
 *
 * Copyright (c) 2017 IUT ANNNECY Departement GEII
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-savoie.fr>
 */

#if !defined __DEVICE_H__
#define      __DEVICE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>

#define SIZESTRV 60

struct TCharDevice {
	char 	m_strFileIO[60];
	int	m_iDescIO;
};

int WriteLcd(int iDescLcd, const char strToLcd[]);
int ReadAdcIA (int iDescAdcIA, char strValue[ ], int iStrSize);
void CheckHandler(int Handler);
void ConvertToVoltage(float* ValPot, float RefVoltage);

#endif // #if !defined __DEVICE_H__
