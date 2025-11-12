/* $Id: device.c, v 1.0 05/07/2017 11:51:52  $
 * modification: 2/10/23
 * linux :  
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modify: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */



#include "device.h"


int WriteLcd(int iDescLcd, const char strToLcd[]) {
	// instructions
	size_t RetWrLcd;
	if (iDescLcd > 0)
	{
		/* code */
		RetWrLcd = write(iDescLcd,strToLcd,strlen(strToLcd));
	}

	else
	{
		switch (iDescLcd)
		{
			case ENODEV:
			/* code */
			fprintf(stderr,"Peripherique introuvable");
			break;
			
			case EBUSY:
			/* code */
			fprintf(stderr,"Peripherique indisponible");
			break;
			
			case ENOMEM:
			/* code */
			fprintf(stderr,"Pas assez de memoires disponible");
			break;
	
			default:
			/* code */
			fprintf(stderr,"une erreur est surevenue");
			break;
		}
	}
	return RetWrLcd;
}
