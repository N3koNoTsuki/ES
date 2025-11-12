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
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>

#include "fctdate.h"
#include "device.h"

/* objet global */
sig_atomic_t sigFlagStop = 0;
sig_atomic_t sigFlagWp = 0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal)
{
	switch (iNumSignal)
	{

	case SIGINT:
		sigFlagStop = -1;
		break;

	case SIGTERM:
		sigFlagStop = -1;
		break;
	case SIGALRM:
		break;

	case SIGUSR1:
		sigFlagWp = 1;
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
	int handler = -1;
	int value = 0;
	pid_t pid;
	int a2iFdPipe[2];
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
		.m_iWBGR = 0x000F000F};

	// Gestion des signaux
	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);
	signal(SIGUSR1, gestionnaire_signal);

	// ouverture driver

	// Main loop
	handler = pipe(a2iFdPipe);
	if (handler == 0)
	{
		pid = fork();

		switch (pid)
		{
		case -1:
			perror("Echec fork()");
			close(a2iFdPipe[0]);
			close(a2iFdPipe[1]);
			break;

		case 0:
			/* code du fils*/
			float ValPot_O = 0;
			cdevAdcIA0.m_iDescIO = open(cdevAdcIA0.m_strFileIO, O_RDWR);
			close(a2iFdPipe[0]);
			ualarm(100000,100000);
			while (sigFlagStop != -1)
			{
				handler = ReadAdcIA(cdevAdcIA0.m_iDescIO, buffer, SIZESTRADC);
				CheckHandler(handler);
				handler = sscanf(buffer, "%f", &ValPot_O);
				// valeur a returner dans le pipe
				handler = write(a2iFdPipe[1], &ValPot_O, sizeof(float));
				CheckHandler(handler);
				kill(getppid(),SIGUSR1);
				pause();
			}
			kill(getppid(),SIGINT);
			close(a2iFdPipe[1]);
			close(cdevAdcIA0.m_iDescIO);
			break;

		default:
			/* code du pere*/
			for (size_t i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					cdevColLedWrgb[i][j].m_iDescIO = open(cdevColLedWrgb[i][j].m_strFileIO, O_RDWR);
					write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_a4cRGBW, 4);
				}
			}
			
			// lire le pipe
			close(a2iFdPipe[1]);
			float ValPot_I;
			float prcnt;
			while (sigFlagStop != -1)
			{
				pause();
				if (sigFlagWp == 1)
				{
					sigFlagWp = 0;
				}
				
				handler = read(a2iFdPipe[0], &ValPot_I, sizeof(float));
				CheckHandler(handler);
				prcnt = 100 * (ValPot_I / 4096);
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
						value = 0xFF * (prcnt / 30);
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
				ConvertToVoltage(&ValPot_I, 5.0, 4096);
				//printf("IA = %.3f V\n", ValPot_I);
			}
			close(a2iFdPipe[0]);
			for (size_t i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					ledBrightNess.m_a4cRGBW[0] = 0x00;
					ledBrightNess.m_a4cRGBW[1] = 0x00;
					ledBrightNess.m_a4cRGBW[2] = 0x00;
					ledBrightNess.m_a4cRGBW[3] = 0x00;

					write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_a4cRGBW, 4);
				}
			}
			for (size_t i = 0; i < 3; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					close(cdevColLedWrgb[i][j].m_iDescIO);
				}
			}
			printf("\n%s>Fin\n", apstrArgValue[0]);

			int status = 0;
			pid_t w = waitpid(pid, &status, 0);
			if (w == -1) {
				perror("waitpid");
			} else {
			}
			break;
		}
	}
	return (iResult);
}
