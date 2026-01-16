/* $Id: oled.c, v 1.0 05/11/2017 11:51:52  $
 * modify : 11/10/23
 *
 * linux : Affiche la tension en sortie du potentiometre sur le flux stdout et le lcd
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modification: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "oled.h"

volatile sig_atomic_t sigFlagStop = 0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal)
{
	if (iNumSignal == SIGINT || iNumSignal == SIGTERM)
	{
		sigFlagStop = -1;
	}
}

int main(int iArgCount, char *apstrArgValue[])
{
	// Clear console at startup
	printf("\033[2J\033[HDebut du programme\n\r");
	fflush(stdout);

	//---------------- Gestion des signaux ----------------//
	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);

	//---------------- Drivers ----------------//
	struct TCharDevice EcranOled = {
		.m_strFileIO = "/dev/ttyAL0",
		.m_iDescIO = -1,
	};

	// Buttons on Falling Edge (o x x o)
	struct TDeviceCharGpIO gpioButton1 = {
		.m_DevChar = {
			.m_iDescIO = -1,
			.m_strFileIO = "/dev/gpiochip2",
		},
	};

	// Buttons on Both Edges (x o o x)
	struct TDeviceCharGpIO gpioButton2 = {
		.m_DevChar = {
			.m_iDescIO = -1,
			.m_strFileIO = "/dev/gpiochip3",
		},
	};
	//---------------- Liaison serie RS-232 ----------------//
	struct termios LiasonOled, LiasonOled_Old = {.c_iflag = 0, .c_oflag = 0, .c_cflag = 0, .c_lflag = 0};

	//---------------- Reservation ----------------//
	int iResult = -1;
	pid_t pid;
	int a2iFdPipe[2];
	char buffer[SIZESTRADC] = {"\0"};

	iResult = pipe(a2iFdPipe);
	if (iResult == 0)
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
			//Le fils lit les boutons est renvois lequelle a etait appuye au pere
			close(a2iFdPipe[0]);	//Ferme la lecture
			config_gpio_button(&gpioButton1, GPIOEVENT_REQUEST_FALLING_EDGE, DEBUG_ERROR);
			config_gpio_button(&gpioButton2, GPIOEVENT_REQUEST_BOTH_EDGES, DEBUG_ERROR);
	
			while (sigFlagStop != -1)
			{
				if (button_press(&gpioButton1, 0) == 1)
				{
					/* code */
					printf("a\n\r");
				}
				if (button_press(&gpioButton1, 1) == 1)
				{
					/* code */
					printf("b\n\r");
				}
				if (button_press(&gpioButton2, 0) == 1)
				{
					/* code */
					printf("c\n\r");
				}
				if (button_press(&gpioButton2, 1) == 1)
				{
					/* code */
					printf("d\n\r");
				}			
			}
			kill(getppid(),SIGINT);
			close(a2iFdPipe[1]);
			close(gpioButton1.m_request.fd);
			close(gpioButton1.m_DevChar.m_iDescIO);
			close(gpioButton2.m_request.fd);
			close(gpioButton2.m_DevChar.m_iDescIO);
			break;

		default:
			/* code du pere*/
			//Le pere lit le pipe est affiche le gif en fonction du bouton appuyer
			close(a2iFdPipe[1]);	//Ferme l'ecriture
			while (sigFlagStop != -1)
			{
				/* code */
			}
			close(a2iFdPipe[0]);
			int status = 0;
			pid_t w = waitpid(pid, &status, 0);
			if (w == -1) {
				perror("waitpid");
			} else {
			}
			fprintf(stdout, "Fin du programme\n\r");
		}
	}
























	/*
	
	//---------------- Initialisation des drivers ----------------//
	EcranOled.m_iDescIO = open(EcranOled.m_strFileIO, O_RDWR);
	if (EcranOled.m_iDescIO < 0)
	{
		perror("open");
		return -1;
	}
	//---------------- Configuration de la liaison RS-232 ----------------//
	iResult = init_liaison(&EcranOled, &LiasonOled, &LiasonOled_Old, sizeof(LiasonOled));
	if (iResult < 0)
	{

		printf("Erreur init_liaison : %i\n\r", iResult);
		return -1;
	}
	printf("Initialisation finished\n\r");

	//----------------Init Media-----------------//
	uint16_t err = 0;
	iResult = init_media(EcranOled.m_iDescIO, &err);
	if (iResult < 0)
	{
		printf("%i\n\r", iResult);
		printf("error init Media : %2x\n\r", err);
		goto cleanup;
	}
	printf("Initialisation media ok\n\r");

	//On se place a indexe +7 qui est l'emplacement du nombre de frame 
	uint8_t cmd_SetByteAdresse[6];
	if (gif == 1)
	{
		cmd_SetByteAdresse[0] = 0xFF;
		cmd_SetByteAdresse[1] = 0xB9;
		cmd_SetByteAdresse[2] = 0x02;
		cmd_SetByteAdresse[3] = 0x00;
		cmd_SetByteAdresse[4] = 0x00;
		cmd_SetByteAdresse[5] = 0x07;
		printf("GIF 1\n\r");
	}
	else
	{
		cmd_SetByteAdresse[0] = 0xFF;
		cmd_SetByteAdresse[1] = 0xB9;
		cmd_SetByteAdresse[2] = 0x02;
		cmd_SetByteAdresse[3] = 0x04;
		cmd_SetByteAdresse[4] = 0x02;
		cmd_SetByteAdresse[5] = 0x07;
		printf("GIF 2\n\r");
	}
	
	iResult = SendCmd(EcranOled.m_iDescIO, cmd_SetByteAdresse, sizeof(cmd_SetByteAdresse));
	if (iResult != 0x06)
	{
		printf("error Set Byte Adresse : %2x\n\r", iResult);
		goto cleanup;
	}

	const uint8_t CmdReadByte[2] = {0xFF, 0xB7};
	uint16_t nb_frame;

	iResult = SendCmd2RetV(EcranOled.m_iDescIO, CmdReadByte, &nb_frame);
	if (iResult < 0)
	{
		printf("error Read Byte : %2x\n\r", iResult);
		goto cleanup;
	}
	printf("Valeur adresse (nb frame): %i\n\n\r", nb_frame);
	
	//On se remet a la bonne adresse pour la video.
	cmd_SetByteAdresse[5] = 0x00;
	
	iResult = SendCmd(EcranOled.m_iDescIO, cmd_SetByteAdresse, sizeof(cmd_SetByteAdresse));
	if (iResult != 0x06)
	{
		printf("error Set Byte Adresse def : %2x\n\r", iResult);
		goto cleanup;
	}

	uint8_t cmd_DisplayVideoFrame[8] = {0xFF, 0xBA, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00};
	
	//---------------- Main loop ----------------//

	while (sigFlagStop != -1)
	{
		//---------------- Animation ----------------//
		
		for (size_t i = 0; i <= nb_frame && sigFlagStop != -1; i++)
		{
			cmd_DisplayVideoFrame[6] = (i >> 8) & 0xFF;
			cmd_DisplayVideoFrame[7] = i & 0xFF;
			printf("\033[F\033[KDisplay Frame : %i\n\r", i);
			iResult = SendCmd(EcranOled.m_iDescIO, cmd_DisplayVideoFrame, sizeof(cmd_DisplayVideoFrame));
			if (iResult != 0x06)
			{
				printf("error Display Video Frame : %2x\n\r", iResult);
				goto cleanup;
			}
			usleep(30000);
		}
		
	}
	printf("Fin Video\n\r");
	//---------------- Setting down ----------------//
cleanup:
	printf("\n\033[F\033[KSetting down\n\r");
	iResult = ClearOled(EcranOled.m_iDescIO);
	if (iResult < 0)
	{
		printf("Error %d clearing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
	}
	iResult = change_baudrate(&EcranOled, &LiasonOled, BAUDRATE_9600, DEBUG_NONE);
	if (iResult < 0)
	{
		return -1;
	}
	
	tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
	if (close(EcranOled.m_iDescIO) == -1)
	{
		printf("Error %d closing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
		return -1;
	}
	*/
	
	return (iResult);

}
