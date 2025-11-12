/* $Id: device.c, v 1.0 05/07/2017 11:51:52  $
 * modification: 2/10/23
 * linux :
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modify: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "device.h"

int WriteLcd(int iDescLcd, const char strToLcd[])
{
	// instructions
	int RetWrLcd;
	RetWrLcd = write(iDescLcd, strToLcd, strlen(strToLcd));
	return RetWrLcd;
}

int ReadAdcIA(int iDescAdcIA, char strValue[], int iStrSize)
{
	// instructions
	int RetWrLcd;
	RetWrLcd = read(iDescAdcIA, strValue, iStrSize);
	return RetWrLcd;
}

void CheckHandler(int Handler)
{
	// instructions

	if (Handler >= 0)
	{
	}

	else
	{
		switch (Handler)
		{
		case ENODEV:
			/* code */
			fprintf(stderr, "Peripherique introuvable\n");
			break;

		case EBUSY:
			/* code */
			fprintf(stderr, "Peripherique indisponible\n");
			break;

		case ENOMEM:
			/* code */
			fprintf(stderr, "Pas assez de memoires disponible\n");
			break;

		default:
			/* code */
			fprintf(stderr, "une erreur est surevenue\n");
			break;
		}
	}
}

void ConvertToVoltage(float *ValPot, float RefVoltage)
{
	*ValPot = (*ValPot / 4096) * RefVoltage;
}

void ClearLastRawShell(void)
{
	printf("\033[A");
	printf("\033[2K");
}

size_t DateLocale(const char *strFormatDate, char *strDate, size_t sztTaillestrDate)
{
	time_t Time = time(NULL);
	struct tm ActualTime = *localtime(&Time);
	return strftime(strDate, SIZESTRDATE, strFormatDate, &ActualTime);
}
