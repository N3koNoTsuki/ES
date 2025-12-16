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

sig_atomic_t sigFlagStop = 0;

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

	default:
		fprintf(stdout, "GstSig : Nothing to do \n");
		break;
	}
}

int main(int iArgCount, char *apstrArgValue[])
{
	printf("\033c");


#pragma region Gestion des signaux
				//---------------- Gestion des signaux ----------------//
	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);
#pragma endregion


#pragma region Drivers
				//---------------- Drivers ----------------//
	struct TCharDevice EcranOled = {
		.m_strFileIO = "/dev/ttyAL0",
		.m_iDescIO = -1,
	};
#pragma endregion


#pragma region Liaison serie RS-232
				//---------------- Liaison serie RS-232 ----------------//
	struct termios LiasonOled, LiasonOled_Old = {.c_iflag = 0, .c_oflag = 0, .c_cflag = 0, .c_lflag = 0};
#pragma endregion


#pragma region Reservation
				//---------------- Reservation ----------------//
	int iResult = 0;
	u_int8_t clear[2] = {0xFF, 0xD7};
	u_int8_t home[6] = {0xFF, 0xE4, 0x00, 0x05, 0x00, 0x03};
	u_int8_t changeColor[6] = {0xFF, 0xBE, 0x00, 0x00, 0x00, 0x1F};
	u_int8_t circle[10] = {0xFF, 0xCD, 0x00, 0x46, 0x00, 0x46, 0x00, 0x14, 0x80, 0x10};
	u_int8_t command[4] = {0xFF, 0xFE, 0x00, 0x39};
	u_int8_t BaudRate115200[4] = {0x00, 0x0B, 0x00, 0x19};
	u_int8_t ack = 0;
#pragma endregion


#pragma region Initialisation des drivers
				//---------------- Initialisation des drivers ----------------//
	EcranOled.m_iDescIO = open(EcranOled.m_strFileIO, O_RDWR | O_NONBLOCK);
	if (EcranOled.m_iDescIO < 0)
	{
		perror("open");
		return -1;
	}
#pragma endregion


#pragma region Configuration de la liaison RS-232 
				//---------------- Configuration de la liaison RS-232 ----------------//
	tcgetattr(EcranOled.m_iDescIO, &LiasonOled);
	memcpy(&LiasonOled_Old, &LiasonOled, sizeof(LiasonOled));
	LiasonOled_Old = LiasonOled; // Buffer pour enregistrer les params par defauts

	cfmakeraw(&LiasonOled);
	LiasonOled.c_cflag |= (CLOCAL | CREAD); // ignore modem control lines and enable receiver
	LiasonOled.c_cflag &= ~PARENB;			// no parity
	LiasonOled.c_cflag &= ~CSTOPB;			// 1 stop bit
	LiasonOled.c_cflag &= ~CSIZE;			// clear data bits setting
	LiasonOled.c_cflag |= CS8;				// 8 data bits
	LiasonOled.c_cc[VTIME] = 1;				// inter-character timer unused
	LiasonOled.c_cc[VMIN] = 0;				// blocking read until 0 character arrives
	iResult = cfsetospeed(&LiasonOled, B9600);
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
	sleep(1);
	iResult = write(EcranOled.m_iDescIO, BaudRate115200, sizeof(BaudRate115200));
	if (iResult < 0)
	{
		perror("Set Baud");
		tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
		close(EcranOled.m_iDescIO);
		return -1;
	}
	sleep(1);
	iResult = cfsetspeed(&LiasonOled, B115200);
	if (iResult < 0)
	{
		perror("cfsetspeed");
		close(EcranOled.m_iDescIO);
		return -1;
	}
	iResult = tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled);
	if (iResult < 0)
	{
		perror("tcsetattr");
		tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
		close(EcranOled.m_iDescIO);
		return -1;
	}
	sleep(1);
	iResult = read(EcranOled.m_iDescIO, &ack, sizeof(ack));
	if (iResult == -1)
	{
		printf("Error %d reading from device %s: %s\n", errno, EcranOled.m_strFileIO, strerror(errno));
	}
	else if (ack == 0x06)
	{
		printf("Ack received from device %s\n", EcranOled.m_strFileIO);
		ack = 0;
	}
	else
	{
		printf("Bad ack received from device %s: 0x%02X\n", EcranOled.m_strFileIO, ack);
		ack = 0;
	}
	if (cfgetospeed(&LiasonOled) == B115200 && cfgetispeed(&LiasonOled) == B115200)
	{
		fprintf(stderr, "baudrate changed to 115200\n");
	}
#pragma endregion
/*
#pragma region Affiche un cercle pour tester			
				//---------------- Affiche un cercle pour tester ----------------//
	iResult = write(EcranOled.m_iDescIO, circle, sizeof(circle));
	if (iResult <0)
	{
		
		perror("Write ");
	}
	iResult = read(EcranOled.m_iDescIO, &ack, sizeof(ack));
	if (iResult <0)
	{
		
		perror("read ");
	}
#pragma endregion
	*/

#pragma region Animation
	while (sigFlagStop != -1)
	{
				//---------------- Animation ----------------//







	}
#pragma endregion


#pragma region Setting down

				//---------------- Setting down ----------------//
	iResult = write(EcranOled.m_iDescIO, clear, sizeof(clear));
	if (iResult == -1)
	{
		printf("\033[2K\rError %d writing to device %s: %s\n\n", errno, EcranOled.m_strFileIO, strerror(errno));
	}
	iResult = read(EcranOled.m_iDescIO, &ack, sizeof(ack));
	if (iResult == -1)
	{
		printf("\033[2K\rError %d reading from device %s: %s\n\n", errno, EcranOled.m_strFileIO, strerror(errno));
	}
	tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
	if (close(EcranOled.m_iDescIO) == -1)
	{
		printf("\033[2K\rError %d closing device %s: %s\n\n", errno, EcranOled.m_strFileIO, strerror(errno));
		return -1;
	}
	fprintf(stdout, "\033[2K\rFin du programme\n\r");
#pragma endregion
	return (iResult);
}
