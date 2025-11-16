/* $Id: device.c, v 1.0 05/07/2017 11:51:52  $
 * modification: 2/10/23
 * linux :  
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modify: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "device.h"

int ReadAdcIA(int iDescAdcIA, char strValue[], int iStrSize) {
	// instructions
	size_t RetWrLcd;
	
	
	if (iDescAdcIA > 0)
	{
		/* code */
		RetWrLcd = read(iDescAdcIA, strValue, iStrSize);
	}

	else
	{
		switch (iDescAdcIA)
		{
			case ENODEV:
			/* code */
			fprintf(stderr,"Peripherique introuvable\n");
			break;
			
			case EBUSY:
			/* code */
			fprintf(stderr,"Peripherique indisponible\n");
			break;
			
			case ENOMEM:
			/* code */
			fprintf(stderr,"Pas assez de memoires disponible\n");
			break;
	
			default:
			/* code */
			fprintf(stderr,"une erreur est surevenue\n");
			break;
		}
	}
	return RetWrLcd;
}

void CheckHandler(int Handler) {
	// instructions
	
	if (Handler > 0)
	{

	}

	else
	{
		switch (Handler)
		{
			case ENODEV:
			/* code */
			fprintf(stderr,"Peripherique introuvable\n");
			break;
			
			case EBUSY:
			/* code */
			fprintf(stderr,"Peripherique indisponible\n");
			break;
			
			case ENOMEM:
			/* code */
			fprintf(stderr,"Pas assez de memoires disponible\n");
			break;
	
			default:
			/* code */
			perror("Erreur ReadAdcIA");
			fprintf(stderr,"une erreur est surevenue\n");
			break;
		}
	}
}

float ConvertToVoltage(int ValPot, float RefVoltage){

	float RetVoltage = 0.0;
	
	RetVoltage = (ValPot / 4096.0) * RefVoltage;

	return RetVoltage;
}

void ClearLastRawShell(void){
	printf("\033[A");  
    printf("\033[2K"); 
}


