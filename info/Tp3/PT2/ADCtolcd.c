/* $Id: tolcd.c, v 1.0 05/12/2017 11:51:52  $
 * modification: 2/10/23
 * linux : Affiche l'heure HH:MN:SS sur l'ecran lcd
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modify: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "device.h"

int main(int iArgCount, char *apstrArgValue[])
{

	/* reservation et initialisation */
	int iResult = 0;
	int handler = -1;
	// Volt
	char buffer[SIZESTRV] = {"\0"};
	float ValPot = 0;

	// Heure
	char OutFormat[20] = {"%T\n"};
	char OutDate[SIZESTRDATE];
	char BufferOutDate[SIZESTRDATE];

	struct TCharDevice cdevLcd = {
		.m_strFileIO = "/dev/lcd162cog0",
		.m_iFdIO = -1,
	};
;	
	struct TCharDevice cdevAdIA0 = {
		.m_strFileIO = "/dev/ad7928c0IA0",
		.m_iFdIO = -1,
	};

	cdevAdIA0.m_iFdIO = open(cdevAdIA0.m_strFileIO, O_RDWR);
	cdevLcd.m_iFdIO = open(cdevLcd.m_strFileIO, O_RDWR);

	while (1)
	{

		if (strcmp(OutDate, BufferOutDate) > 0)
		{
			// Acquisition heure
			DateLocale(OutFormat, OutDate, SIZESTRDATE);
			strcpy(BufferOutDate, OutDate);
			
			// Acquisition Volt
			handler = ReadAdcIA(cdevAdIA0.m_iFdIO, buffer, SIZESTRV);
			
			handler = sscanf(buffer, "%f", &ValPot);
			ConvertToVoltage(&ValPot, 5.0);
			
			handler = sprintf(buffer, "IAO = %.3f V", ValPot);
			
			// Affichage
			ioctl(cdevLcd.m_iFdIO, LCD_CLEAR_DISPLAY);
			handler = WriteLcd(cdevLcd.m_iFdIO, strcat(OutDate, buffer));
			CheckHandler(handler);
			
			usleep(250000);
		}
		else
		{
			DateLocale(OutFormat, OutDate, SIZESTRDATE);
		}
	}
	return (iResult);

}
