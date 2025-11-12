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
	int lcd_fd = open("/dev/lcd162cog0", O_RDWR);
	/* reservation et initialisation */
	int iResult = 0;
	
	/* traitement */

	WriteLcd(lcd_fd, "HELLO GEII!!");
	
	close(lcd_fd);
	return (iResult);
}



