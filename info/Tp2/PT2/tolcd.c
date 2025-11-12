/* $Id: tolcd.c, v 1.0 05/12/2017 11:51:52  $
 * modification: 2/10/23
 * linux : Affiche l'heure HH:MN:SS sur l'ecran lcd
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modify: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */



#include "device.h"







int main(int iArgCount, char* apstrArgValue[]){
	// reservation
	int lcd_fd = open("/dev/lcd162cog0", O_RDWR);
	int iResult=0;
		
	char OutFormat[20] = {"%T\n%D"};
	char OutDate[SIZESTRDATE];
	char BufferOutDate[SIZESTRDATE];

	DateLocale(OutFormat, OutDate, SIZESTRDATE);
	ioctl(lcd_fd,LCD_CLEAR_DISPLAY);
	WriteLcd(lcd_fd,OutDate);
	strcpy(BufferOutDate, OutDate);

	while (1)
	{
		
		if (strcmp(OutDate, BufferOutDate) > 0)
		{
			DateLocale(OutFormat, OutDate, SIZESTRDATE); 
			fflush(stdout); 
			ioctl(lcd_fd,LCD_CLEAR_DISPLAY);
			WriteLcd(lcd_fd,OutDate);
			strcpy(BufferOutDate, OutDate);
			sleep(1);
		}
		else{
			DateLocale(OutFormat, OutDate, SIZESTRDATE);
		}
	}
	close(lcd_fd);
	return (iResult);
}


