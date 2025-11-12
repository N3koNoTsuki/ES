/* $Id: device.c, v 1.0 05/07/2017 11:51:52  $
 *
 * linux :
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 */
#include "device.h"

int ReadAdcIA(int iDescAdcIA, char strValue[], int iStrSize)
{

	/* reservation et initialisation */
	long iResult = 0;
	size_t sztNbrBytesRead = 0;

	sztNbrBytesRead = 0;
	while ((sztNbrBytesRead < iStrSize) && (iResult >= 0))
	{
		iResult = read(iDescAdcIA, &strValue[sztNbrBytesRead], (iStrSize - sztNbrBytesRead));
		if (iResult >= 0)
		{
			sztNbrBytesRead = sztNbrBytesRead + iResult;
		}
		else
		{
			perror("read AdcIA:");
		}
	}
	if (sztNbrBytesRead == iStrSize)
	{
		iResult = sztNbrBytesRead;
	}
	return (iResult);
}

int WriteLcd(int iDescLcd, const char strToLcd[])
{

	/* reservation et initialisation */
	int iResult = 0;
	size_t sztNbrBytesToWrite = 0;
	size_t sztNbrBytesWritten = 0;

	/* instructions */
	iResult = 0;
	sztNbrBytesWritten = 0;
	sztNbrBytesToWrite = strlen(strToLcd);
	while ((sztNbrBytesWritten < sztNbrBytesToWrite) && (iResult >= 0))
	{
		iResult = write(iDescLcd, &strToLcd[sztNbrBytesWritten], (sztNbrBytesToWrite - sztNbrBytesWritten));
		if (iResult >= 0)
		{
			sztNbrBytesWritten = sztNbrBytesWritten + iResult;
		}
		else
		{
			perror("write lcd:");
		}
	}
	if (sztNbrBytesWritten == sztNbrBytesToWrite)
	{
		iResult = sztNbrBytesWritten;
	}

	return (iResult);
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

void ConvertToVoltage(float *ValPot, float RefVoltage, int resolution)
{
	*ValPot = (*ValPot / resolution) * RefVoltage;
}
