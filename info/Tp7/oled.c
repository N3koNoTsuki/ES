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
	uint8_t BaudRate115200[4] = {0x00, 0x0B, 0x00, 0x19};

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
	sleep(1);;
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
	uint16_t media_ok = 0;
	if (goldelox_media_init(EcranOled.m_iDescIO, &media_ok) < 0)
	{
		perror("goldelox_media_init");
		goto cleanup;
	}
	if (media_ok == 0)
	{
		fprintf(stderr, "Media Init: pas de carte ou init impossible (value=0)\n\r"); // 
		goto cleanup;
	}
	const uint32_t VIDEO_BASE_SECTOR = 4096; // secteur où l'image/vidéo est écrite (dd seek)
	if (goldelox_set_sector_address(EcranOled.m_iDescIO, VIDEO_BASE_SECTOR) < 0)
	{
		perror("goldelox_set_sector_address");
		goto cleanup;
	}

	const uint16_t X0 = 0;        // TODO: position écran
	const uint16_t Y0 = 0;        // TODO: position écran
	const uint16_t NB_FRAMES = 1; // nombre de frames disponibles dans le .raw
	const int FRAME_DELAY_US = 15000; // ~15 ms => ~66 fps (à ajuster)

	uint16_t frame = 0;

	while (sigFlagStop != -1)
	{
		//---------------- Animation ----------------//
		if (goldelox_video_frame_raw(EcranOled.m_iDescIO, X0, Y0, frame) < 0)
		{
			perror("goldelox_video_frame_raw");
			break;
		}

		frame++;
		if (frame >= NB_FRAMES) frame = 0;

		usleep(FRAME_DELAY_US);

	}

	//---------------- Setting down ----------------//
cleanup:
	printf("\n\033[F\033[KSetting down\n\r");
	iResult = CleaOled(EcranOled.m_iDescIO);
	if (iResult < 0)
	{
		printf("Error %d clearing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
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
