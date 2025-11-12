/* $Id: divice.h, v 1.0 05/07/2009 11:51:52  $
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
#include <linux/ioctl.h>

// magic number: 'l'
#define KLCD162COG_NBR_MAGIQUE 'l'

// commande ioctl
#define LCD_CURSOR_HOME         _IO (KLCD162COG_NBR_MAGIQUE, 0x10)
#define LCD_CLEAR_DISPLAY       _IO (KLCD162COG_NBR_MAGIQUE, 0x11)
#define LCD_CURSOR_DISPLAY      _IO (KLCD162COG_NBR_MAGIQUE, 0x12)
#define LCD_CURSOR_NODISPLAY    _IO (KLCD162COG_NBR_MAGIQUE, 0x13)
#define LCD_CURSOR_BLINKING     _IO (KLCD162COG_NBR_MAGIQUE, 0x14)
#define SIZESTRDATE 80
#define SIZESTRV 80


struct TCharDevice{
	char m_strFileIO[60];
	int m_iFdIO;
};

int WriteLcd(int iDescLcd, const char strToLcd[]);
int ReadAdcIA(int iDescAdcIA, char strValue[], int iStrSize);
void CheckHandler(int Handler);
void ConvertToVoltage(float* ValPot, float RefVoltage);
void ClearLastRawShell(void);
size_t DateLocale(const char* strFormatDate, char* strDate, size_t sztTaillestrDate);


#endif // #if !defined __DEVICE_H__
