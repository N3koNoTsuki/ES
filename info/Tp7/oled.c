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

	//---------------- Liaison serie RS-232 ----------------//
	struct termios LiasonOled, LiasonOled_Old = {.c_iflag = 0, .c_oflag = 0, .c_cflag = 0, .c_lflag = 0};

	//---------------- Reservation ----------------//
	int iResult = 0;

	//---------------- Initialisation des drivers ----------------//
	EcranOled.m_iDescIO = open(EcranOled.m_strFileIO, O_RDWR);
	if (EcranOled.m_iDescIO < 0)
	{
		perror("open");
		return -1;
	}
	//---------------- Configuration de la liaison RS-232 ----------------//
	if (tcgetattr(EcranOled.m_iDescIO, &LiasonOled) < 0)
	{
		perror("tcgetattr");
		close(EcranOled.m_iDescIO);
		return -1;
	}
	memcpy(&LiasonOled_Old, &LiasonOled, sizeof(LiasonOled));
	LiasonOled_Old = LiasonOled; // Buffer pour enregistrer les params par defauts
	cfmakeraw(&LiasonOled);
	LiasonOled.c_cflag |= (CLOCAL | CREAD); // ignore modem control lines and enable receiver
	LiasonOled.c_cflag &= ~PARENB;			// no parity
	LiasonOled.c_cflag &= ~CSTOPB;			// 1 stop bit
	LiasonOled.c_cflag &= ~CSIZE;			// clear data bits setting
	LiasonOled.c_cflag |= CS8;				// 8 data bits
	LiasonOled.c_cc[VTIME] = 1;
	LiasonOled.c_cc[VMIN] = 0;
	iResult = cfsetspeed(&LiasonOled, B9600);
	if (iResult < 0)
	{
		perror("cfsetspeed");
		close(EcranOled.m_iDescIO);
		return -1;
	}
	iResult = tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled);
	if (iResult < 0)
	{
		tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old);
		perror("tcsetattr");
		close(EcranOled.m_iDescIO);
		return -1;
	}
	if (fcntl(EcranOled.m_iDescIO, F_SETFL, O_RDWR))
	{
		printf("Error %d setting device %s flags: %s\n", errno, EcranOled.m_strFileIO, strerror(errno));
		tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
		close(EcranOled.m_iDescIO);
		return -1;
	}
	sleep(3);
	
	iResult = change_baudrate(EcranOled, &LiasonOled, BAUDRATE_115200, DEBUG_NONE);
	if (iResult < 0)
	{
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

	const uint8_t cmd_SetByteAdresse[6] = {0xFF, 0xB9, 0x00, 0x20, 0x00, 0x00};
	iResult = SendCmd(EcranOled.m_iDescIO, cmd_SetByteAdresse, sizeof(cmd_SetByteAdresse));
	if (iResult < 0)
	{
		printf("error Set Byte Adresse : %2x\n\r", iResult);
		goto cleanup;
	}

	const uint8_t cmd_DisplayVideoRaw[6] = {0xFF, 0xBB, 0x00, 0x00, 0x00, 0x00};
	iResult = SendCmd(EcranOled.m_iDescIO, cmd_DisplayVideoRaw, sizeof(cmd_DisplayVideoRaw));
	if (iResult < 0)
	{
		printf("error Display Video Raw : %2x\n\r", iResult);
		goto cleanup;
	}
	
	printf("Fin Video\n\r");


	//---------------- Main loop ----------------//

	while (sigFlagStop != -1)
	{
		//---------------- Animation ----------------//


	}

	//---------------- Setting down ----------------//
cleanup:
	printf("\n\033[F\033[KSetting down\n\r");
	iResult = ClearOled(EcranOled.m_iDescIO);
	if (iResult < 0)
	{
		printf("Error %d clearing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
	}
	iResult = change_baudrate(EcranOled, &LiasonOled, BAUDRATE_9600, DEBUG_NONE);
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
	fprintf(stdout, "Fin du programme\n\r");
	return (iResult);
}
