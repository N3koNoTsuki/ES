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

	int iResult = 0;
	int handler = 0;
	char buffer[10] = {" "};
	int ValPot = -1;

	struct TCharDevice cdevAdIA0 = {
		.m_strFileIO = "/dev/ad7928c0IA0",
		.m_iFdIO = -1,
	};
	
	cdevAdIA0.m_iFdIO = open(cdevAdIA0.m_strFileIO, O_RDWR);
	
	while (1)
	{
		handler = ReadAdcIA(cdevAdIA0.m_iFdIO, buffer, sizeof(buffer));
		CheckHandler(handler);
		handler = sscanf(buffer , "%i" , &ValPot);
		CheckHandler(handler);
		ClearLastRawShell();
		printf("%4.3f\n",ConvertToVoltage(ValPot, 2.5));
		sleep(1);
	}
	return (iResult);
}



