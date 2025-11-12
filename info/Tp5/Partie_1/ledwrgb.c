/* $Id: som.c, v 1.0 05/11/2017 11:51:52  $
 *							04/11/2023
 *
 * linux : 1) Affiche la tension en sortie du potentiometre sur le flux stdout
 *			  2) Animation des leds wrgb
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * Co-author : Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#undef __DEBUG_INFO__
#define __DEBUG_INFO__
#undef __DEBUG_INFO__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>

#include "fctdate.h"
#include "device.h"

/* objet global */
sig_atomic_t sigFlagStop = 0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal)
{
	switch (iNumSignal) {

		case SIGINT:
			sigFlagStop = -1;
			break;

		case SIGTERM:
			sigFlagStop = -1;
			break;
		case SIGALRM:
			break;

		default:
			fprintf(stdout, "GstSig : Nothing to do \n");
			break;
	}
}



int main(int iArgCount, char *apstrArgValue[])
{
	
	
	/* reservation et initialisation */
	int iResult = 0;
	float ValPot = 0;
	int handler = -1;
	int value = 0;
	char buffer[SIZESTRADC] = {"\0"};

	struct TCharDevice cdevAdcIA0 = {
		.m_strFileIO = "/dev/ad7928c0IA0",
		.m_iDescIO = -1,
	};
	struct TCharDevice cdevColLedWrgb[3][3] = {
		{
			{
				.m_strFileIO = "/dev/ledrgbw6",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw3",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw0",
				.m_iDescIO = -1,
			},
		},
		{
			{
				.m_strFileIO = "/dev/ledrgbw7",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw4",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw1",
				.m_iDescIO = -1,
			},
		},
		{
			{
				.m_strFileIO = "/dev/ledrgbw8",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw5",
				.m_iDescIO = -1,
			},
			{
				.m_strFileIO = "/dev/ledrgbw2",
				.m_iDescIO = -1,
			},
		},
	};
	union TLedBrightNess ledBrightNess = {
		.m_iWBGR = 0x000F000F
	};

	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			cdevColLedWrgb[i][j].m_iDescIO = open(cdevColLedWrgb[i][j].m_strFileIO, O_RDWR);
			write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_a4cRGBW, 4);
		}
	}

	cdevAdcIA0.m_iDescIO = open(cdevAdcIA0.m_strFileIO, O_RDWR);

	ledBrightNess.m_iWBGR = 0x000F000F;
	write(cdevColLedWrgb[1][1].m_iDescIO, &ledBrightNess.m_iWBGR, 4);

	while(sigFlagStop != -1 )
	{
		handler = ReadAdcIA(cdevAdcIA0.m_iDescIO, buffer, SIZESTRADC);
		CheckHandler(handler);
		handler = sscanf(buffer, "%f", &ValPot);
		/*
		float prcnt = 100 * (ValPot/4096);
		float valueMax = 0xFF * (prcnt/100.0);
		float decrease = valueMax/9; //nb de leds
		for (size_t i = 0; i < 3; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				int Value = valueMax - (decrease * (3*i+j));
				ledBrightNess.m_a4cRGBW[0] = 0x00;
				ledBrightNess.m_a4cRGBW[1] = 0x00;
				ledBrightNess.m_a4cRGBW[2] = Value;
				ledBrightNess.m_a4cRGBW[3] = 0x00;

				write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_a4cRGBW, 4);
			}
			
		}
		*/
		float prcnt = 100 * (ValPot/4096);

		for (size_t i = 0; i < 3; i++)
		{
			if (prcnt > 30)
			{
				value = 0xFF;
			}
			else if (prcnt < 0)
			{
				value = 0x00;
			}
			else
			{
				value = 0xFF * (prcnt/30);
			}
			prcnt -= 30;
			for (size_t j = 0; j < 3; j++)
			{
				ledBrightNess.m_a4cRGBW[0] = 0x00;
				ledBrightNess.m_a4cRGBW[1] = 0x00;
				ledBrightNess.m_a4cRGBW[2] = value;
				ledBrightNess.m_a4cRGBW[3] = 0x00;

				write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_a4cRGBW, 4);
			}
			
		}






		ConvertToVoltage(&ValPot, 5.0, 4096);
		handler = sprintf(buffer, "IA = %.3f V\n", ValPot);
		printf(buffer);
	}
	



	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			close(cdevColLedWrgb[i][j].m_iDescIO);
		}
	}
	close(cdevAdcIA0.m_iDescIO);
	printf("%s>Fin :\n", apstrArgValue[0]);
	return (iResult);
}
