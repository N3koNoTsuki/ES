/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * linux : Affiche l'heure HH:MN sur le flux stdout
 *
 * Copyright (c) 2024 IUT ANNNECY Departement GEII/ESE
 * Author: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */
 
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "info.h"


//#define CahierCharges_1			// Affichage de 20 secondes sur le hex7seg2
//#define CahierCharges_2			// Compte a rebours sur le hex7seg2 de 20 a 0
//#define CahierCharges_3			// Compte a rebours sur le hex7seg2 de 20 a 0 et animation des leds wrgb a partir de 6
//#define CahierCharges_4				// Compte a rebours sur le hex7seg2 de n a 0 et animation des leds wrgb a partir de 6 avec n en argument
#define CahierCharges_5			// Processus pere fils pour la gestion du compte a rebours et de l'animation des leds wrgb

#ifdef CahierCharges_1
#undef CahierCharges_5
#undef CahierCharges_4
#undef CahierCharges_3
#undef CahierCharges_2
#endif

#ifdef CahierCharges_2
#undef CahierCharges_5
#undef CahierCharges_4
#undef CahierCharges_3
#undef CahierCharges_1
#endif

#ifdef CahierCharges_3
#define CahierCharges_2
#undef CahierCharges_5
#undef CahierCharges_4
#undef CahierCharges_1
#endif

#ifdef CahierCharges_4
#define CahierCharges_3
#define CahierCharges_2
#undef CahierCharges_5
#undef CahierCharges_1
#endif

#ifdef CahierCharges_5
#undef CahierCharges_4
#undef CahierCharges_3
#undef CahierCharges_2
#undef CahierCharges_1
#endif



sig_atomic_t sigFlagStop = 0;

// signal handler
static void gestionnaire_signal(int iSig){
	switch (iSig) {

		case SIGINT:
			sigFlagStop = -1;
			break;

		case SIGTERM:
			sigFlagStop = -1;
			break;

		case SIGALRM:
			break;

		case SIGUSR1:
		break;

		default:
			fprintf(stdout, "GstSig : Nothing to do \n");
			break;
	}
}

int main(int iArgCount, char* apstrArgValue[]){

	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);
	signal(SIGUSR1, gestionnaire_signal);
	// reservation
	int iResult = 0;

	struct TCharDevice cdevhex7seg = {
			.m_strFileIO 	= "/dev/hex7seg2",
			.m_iDescIO 		= -1,
	};

	struct TCharDevice cdevColLedWrgb[3][3] = {	{	{	.m_strFileIO 	= "/dev/ledrgbw6",	.m_iDescIO 		= -1,	},																
													{	.m_strFileIO 	= "/dev/ledrgbw3",	.m_iDescIO 		= -1,	},
													{	.m_strFileIO 	= "/dev/ledrgbw0",	.m_iDescIO 		= -1,	},	},
												{	{	.m_strFileIO 	= "/dev/ledrgbw7",	.m_iDescIO 		= -1,	},
													{	.m_strFileIO 	= "/dev/ledrgbw4",	.m_iDescIO 		= -1,	},
													{	.m_strFileIO 	= "/dev/ledrgbw1",	.m_iDescIO 		= -1,	},	},
												{	{	.m_strFileIO 	= "/dev/ledrgbw8",	.m_iDescIO 		= -1,	},
													{	.m_strFileIO 	= "/dev/ledrgbw5",	.m_iDescIO 		= -1,	},
													{	.m_strFileIO 	= "/dev/ledrgbw2",	.m_iDescIO 		= -1,	},	},
	};
	
	union TLedBrightNess ledBrightNess = {
			.m_iWBGR	=	0,
	};
	
	const int iFace[7][9] = 	{	{  0,  0,  0,  0,  0,  0,  0,  0,  0},
									{  0,  0,  0,  0,255,  0,  0,  0,  0},
									{255,  0,  0,  0,  0,  0,  0,  0,255},
									{255,  0,  0,  0,255,  0,  0,  0,255},
									{255,  0,255,  0,  0,  0,255,  0,255},
									{255,  0,255,  0,255,  0,255,  0,255},
									{255,  0,255,255,  0,255,255,  0,255}
	};
	
	int8_t Value = 0x20;
	//Initialisation des drivers
	cdevhex7seg.m_iDescIO = open(cdevhex7seg.m_strFileIO,O_RDWR);
	if (cdevhex7seg.m_iDescIO < 0) {
		perror("open ADC ");
	}

	#ifdef CahierCharges_3 
	ledBrightNess.m_iWBGR = 0x00000000; 

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			cdevColLedWrgb[i][j].m_iDescIO = open(cdevColLedWrgb[i][j].m_strFileIO, O_RDWR);
			if (cdevColLedWrgb[i][j].m_iDescIO < 0) {
				perror("open LED ");
			}
			iResult = write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_iWBGR, 4);
			if (iResult < 0) {
				perror("write LED ");
			}
		}
	}
	#endif
	#ifdef CahierCharges_5
	ledBrightNess.m_iWBGR = 0x00000000; 

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			cdevColLedWrgb[i][j].m_iDescIO = open(cdevColLedWrgb[i][j].m_strFileIO, O_RDWR);
			if (cdevColLedWrgb[i][j].m_iDescIO < 0) {
				perror("open LED ");
			}
			iResult = write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_iWBGR, 4);
			if (iResult < 0) {
				perror("write LED ");
			}
		}
	}
	#endif

	#ifdef CahierCharges_1
	//Ecriture valeur
	iResult = write(cdevhex7seg.m_iDescIO, &Value, sizeof(Value));
	if (iResult < 0) {
		perror("write hex7seg ");
	}

	alarm(2);
	pause();
	#endif

	#ifdef CahierCharges_2
	//Boucle d'ecriture
	#ifndef CahierCharges_4
	__uint8_t Max = 10;
	#else
	__uint8_t Max = apstrArgValue[1] ? (__uint8_t)atoi(apstrArgValue[1]) : 10;
	#endif

	for (size_t i = 0; i < Max + 1; i++)
	{
		/* code */
		//Ecriture valeur
		u_int8_t ActualVal = Max - i;
		if (sigFlagStop == -1)
		{
			/* code */
			break;
		}
		
		Value = ((ActualVal)/10) *16 + ((ActualVal)%10);
		iResult = write(cdevhex7seg.m_iDescIO, &Value, sizeof(Value));
		if (iResult < 0) {
			perror("write hex7seg ");	
		}


		if(ActualVal <= 6) {
			for (size_t j = 0; j < 9; j++)
			{
				if (sigFlagStop == -1)
				{
					break;
				}
				
				ledBrightNess.m_iWBGR  = iFace[ActualVal][j];
				iResult = write(cdevColLedWrgb[j/3][j%3].m_iDescIO, &ledBrightNess.m_iWBGR, 4);
				if (iResult < 0) {
					perror("write LED ");

				}

			}
		} 

		if (i == 20) {
			alarm(2);
			pause();
		}
		else
		{
			alarm(1);
			pause();
		}
	}
	#endif
	

	#ifdef CahierCharges_5

	//procesus pere fils
	pid_t pid;

	pid = fork();

	switch (pid)
	{
	case -1:
		perror("Echec fork()");
		break;

	case 0:
		/* code du fils*/
		ualarm(800000,800000);
		while (sigFlagStop != -1)
		{
			pause();
			kill(getppid(),SIGUSR1);
		}
		kill(getppid(),SIGINT);
		break;
		

	default:
		/* code du pere*/

	__uint8_t Max = apstrArgValue[1] ? (__uint8_t)atoi(apstrArgValue[1]) : 10;


	for (size_t i = 0; i < Max + 1; i++)
	{
		/* code */
		//Ecriture valeur
		u_int8_t ActualVal = Max - i;
		if (sigFlagStop == -1)
		{
			/* code */
			break;
		}
		
		Value = ((ActualVal)/10) *16 + ((ActualVal)%10);
		iResult = write(cdevhex7seg.m_iDescIO, &Value, sizeof(Value));
		if (iResult < 0) {
			perror("write hex7seg ");	
		}


		if(ActualVal <= 6) {
			for (size_t j = 0; j < 9; j++)
			{
				if (sigFlagStop == -1)
				{
					break;
				}
				
				ledBrightNess.m_iWBGR  = iFace[ActualVal][j];
				iResult = write(cdevColLedWrgb[j/3][j%3].m_iDescIO, &ledBrightNess.m_iWBGR, 4);
				if (iResult < 0) {
					perror("write LED ");

				}

			}
		} 

		pause();
	}
		int status = 0;
		pid_t w = waitpid(pid, &status, 0);
		if (w == -1) {
			perror("waitpid");
		} else {
		}
		break;
	}
	
	#endif

















	
	//Fermeture du driver
	iResult = close(cdevhex7seg.m_iDescIO);
	if (iResult < 0) {
		perror("close hex7seg ");

	}
	#ifdef CahierCharges_3
	ledBrightNess.m_iWBGR = 0x00000000; 
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			iResult = write(cdevColLedWrgb[i][j].m_iDescIO, &ledBrightNess.m_iWBGR, 4);
			if (iResult < 0) {
				perror("write LED ");
			}
			iResult = close(cdevColLedWrgb[i][j].m_iDescIO);
			if (iResult < 0) {
				perror("close LED ");
			}
		}
	}
	#endif
	
	fprintf(stdout,"\033[2K\rFin du programme\n\r");
	return iResult;
}
