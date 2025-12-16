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

sig_atomic_t sigFlagStop=0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal) {

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

int main(int iArgCount, char* apstrArgValue[]){
	printf("\033c");
	//Gestion des signaux
	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);
	
	//drivers
	struct TCharDevice EcranOled = {
			.m_strFileIO 	= "/dev/ttyAL0",
			.m_iDescIO 		= -1,
	};

	//Liaison serie RS-232
	struct termios LiasonOled;
	struct termios LiasonOled_Old;


	//reservation	
	int iResult = 0;
	uint8_t cmd_set_speed[] = {0x00, 0x0B, 0x00, 0x19};
	uint8_t cmd[] = {0xFF, 0xCF, 0x00, 0x0A, 0x00, 0x14, 0x00, 0x50, 0x00, 0x50, 0x00, 0x1F};
	#ifdef DEBUG
	fprintf(stdout, "H0\n\r");
	#endif
	//Initialisation des drivers
	EcranOled.m_iDescIO = open(EcranOled.m_strFileIO,O_RDWR);
	#ifdef DEBUG
	perror("open ");
	fprintf(stdout, "H01\n\r");
	#endif
	// Evite de bloquer indéfiniment si aucune donnée n'arrive
	fcntl(EcranOled.m_iDescIO, F_SETFL, O_NONBLOCK);
	//Configuration de la liaison RS-232
	tcgetattr(EcranOled.m_iDescIO,&LiasonOled);
	LiasonOled_Old = LiasonOled;	//Buffer pour enregistrer les params par defauts
	#ifdef DEBUG
	fprintf(stdout, "H02\n\r");
	#endif
	cfmakeraw(&LiasonOled);			// Passe en mode raw et nettoie les flags inutiles
	cfsetispeed(&LiasonOled, B9600);
	cfsetospeed(&LiasonOled, B9600);
	LiasonOled.c_cflag |= CLOCAL | CREAD | CS8;	// Garde la réception active en 8N1
	LiasonOled.c_cc[VMIN]  = 0;		// read ne bloque pas si rien n'arrive
	LiasonOled.c_cc[VTIME] = 5;		// timeout de 0,5 s
	#ifdef DEBUG
	fprintf(stdout, "H03\n\r");
	#endif
	iResult = tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled);
	if (iResult < 0)
	{
		/* code */
		perror("tcsetattr");
	}
	tcflush(EcranOled.m_iDescIO, TCIOFLUSH);

	#ifdef DEBUG
	fprintf(stdout, "H04\n\r");
	#endif

	iResult = write(EcranOled.m_iDescIO, cmd_set_speed, sizeof(cmd_set_speed));
	if (iResult < 0)
	{
		/* code */
		perror("Set Baud");
	}
	#ifdef DEBUG
	fprintf(stdout, "H05\n\r");
	#endif
	usleep(100000); 
	cfsetispeed(&LiasonOled, B115200);
	cfsetospeed(&LiasonOled, B115200);
	tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled);
	tcflush(EcranOled.m_iDescIO, TCIOFLUSH);
	#ifdef DEBUG
	fprintf(stdout, "H06\n\r");
	#endif
	char ack[20];
	iResult = read(EcranOled.m_iDescIO, ack, sizeof(ack));
	if (iResult < 0)
	{
		/* code */
		perror("read ");
	}
	
	#ifdef DEBUG
	fprintf(stdout, "H07\n\r");
	#endif

	iResult = SendCmd(EcranOled.m_iDescIO, cmd, sizeof(cmd));
	#ifdef DEBUG
	fprintf(stdout, "H08\n\r");
	#endif
	clearcmd(cmd, sizeof(cmd));


	while (sigFlagStop != -1)
	{
	
	}
	
	//On clear l'ecran
	iResult = CleaOled(EcranOled.m_iDescIO);

	//On reinitialise la liaison a ses parametre par defaut et on vide le buffer
	//avant de fermer le driver
	tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old);
	tcflush(EcranOled.m_iDescIO, TCIOFLUSH);
	iResult = close(EcranOled.m_iDescIO);

	fprintf(stdout,"\033[2K\rFin du programme\n\r");
	return (iResult);
}
