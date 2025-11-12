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

#define SIZESTRADC 60


struct TrgbwBrightNess {
   char m_cRed;
   char m_cGreen;
   char m_cBlue;
   char m_cWhite;
};

union TLedBrightNess {
	struct TrgbwBrightNess m_rgbwBrightNess;
	char m_a4cRGBW[4];
	int m_iWBGR;
};



struct TCharDevice {
	char m_strFileIO[60];
	int	m_iDescIO;
};

int WriteLcd(int iDescLcd, const char strToLcd[]);
int ReadAdcIA (int iDescAdcIA, char strValue[ ], int iStrSize);
void CheckHandler(int Handler);
void ConvertToVoltage(float *ValPot, float RefVoltage, int resolution);


#endif // #if !defined __DEVICE_H__
