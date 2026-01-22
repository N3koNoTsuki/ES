/* $Id: DIsplayGif.c, v 1.0 05/11/2017 11:51:52  $
 * modify : 11/10/23
 *
 * linux : Affiche la tension en sortie du potentiometre sur le flux stdout et le lcd
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modification: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "DIsplayGif.h"

volatile sig_atomic_t sigFlagStop = 0;
sig_atomic_t sigUser1 = 0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal)
{
	if (iNumSignal == SIGINT || iNumSignal == SIGTERM)
	{
		sigFlagStop = -1;
	}
	if (iNumSignal == SIGUSR1)
	{
		/* code */
		sigUser1 = 1;
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
	signal(SIGUSR1, gestionnaire_signal);

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
			// Le fils lit les boutons est renvois lequelle a etait appuye au pere
			close(a2iFdPipe[0]); // Ferme la lecture

			//---------------- Drivers ----------------//

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

			//---------------- Initialisation des drivers ----------------//
			config_gpio_button(&gpioButton1, GPIOEVENT_REQUEST_FALLING_EDGE, DEBUG_ERROR);
			config_gpio_button(&gpioButton2, GPIOEVENT_REQUEST_BOTH_EDGES, DEBUG_ERROR);
			//---------------- Main loop ----------------//
			while (sigFlagStop != -1)
			{
				iResult = check_all_buttons(&gpioButton1, &gpioButton2);
				if (iResult < 0)
				{
					// No buton press
				}
				else
				{
					buffer[0] = iResult;
					write(a2iFdPipe[1], buffer, 1);
					kill(getppid(), SIGUSR1);
				}
			}
			//---------------- Setting down ----------------//
			kill(getppid(), SIGINT);
			close(a2iFdPipe[1]);
			close(gpioButton1.m_request.fd);
			close(gpioButton1.m_DevChar.m_iDescIO);
			close(gpioButton2.m_request.fd);
			close(gpioButton2.m_DevChar.m_iDescIO);
			printf("\n\033[F\033[KFils end\n\r");
			break;

		default:
			/* code du pere*/
			// Le pere lit le pipe est affiche le gif en fonction du bouton appuyer
			close(a2iFdPipe[1]); // Ferme l'ecriture

			//---------------- Drivers ----------------//
			struct TCharDevice EcranOled = {
				.m_strFileIO = "/dev/ttyAL0",
				.m_iDescIO = -1,
			};

			//---------------- Liaison serie RS-232 ----------------//
			struct termios LiasonOled, LiasonOled_Old = {.c_iflag = 0, .c_oflag = 0, .c_cflag = 0, .c_lflag = 0};

			//---------------- Reservation ----------------//
			int gif = 0;
			int status = 0;
			uint16_t err = 0;
			uint16_t nb_frame;
			pid_t w;

			uint8_t cmd_DisplayVideoFrame[8] = {0xFF, 0xBA, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00};
			uint8_t (*cmd_SetByteAdresse)[6] = NULL;
			size_t gif_count = 0;

			//---------------- Initialisation des drivers ----------------//
			EcranOled.m_iDescIO = open(EcranOled.m_strFileIO, O_RDWR);
			if (EcranOled.m_iDescIO < 0)
			{
				perror("open");
				goto cleanup;
			}

			//---------------- Configuration de la liaison RS-232 ----------------//
			iResult = init_liaison(&EcranOled, &LiasonOled, &LiasonOled_Old, sizeof(LiasonOled));
			if (iResult < 0)
			{

				printf("Erreur init_liaison : %i\n\r", iResult);
				goto cleanup;
			}
			printf("Initialisation finished\n\r");

			//----------------Init Media-----------------//

			iResult = init_media(EcranOled.m_iDescIO, &err);
			if (iResult < 0)
			{
				printf("%i\n\r", iResult);
				printf("error init Media : %2x\n\r", err);
				goto cleanup;
			}
			printf("Initialisation media ok\n\r");

			// Charge la table des adresses GIF depuis le fichier.
			if (LoadGifAddresses(GIF_ADDRESS_PATH, &cmd_SetByteAdresse, &gif_count) != 0)
			{
				fprintf(stderr, "Erreur lecture adresses: %s\n", GIF_ADDRESS_PATH);
				goto cleanup;
			}
			if (gif_count == 0)
			{
				fprintf(stderr, "Aucune adresse GIF valide dans %s\n", GIF_ADDRESS_PATH);
				free(cmd_SetByteAdresse);
				goto cleanup;
			}

			printf("=======================\n\r");
			printf("Playing gif nb : %i\n\r", gif);
			iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse[gif], &nb_frame);
			if (iResult < 0)
			{
				printf("error Get Nb Frames : %2x\n\r", iResult);
				goto cleanup;
			}
			printf("Valeur adresse (nb frame): %u\n\n\n\r", nb_frame);
			//---------------- Main loop ----------------//
			while (sigFlagStop != -1)
			{
				/* code */
				if (sigUser1 == 1)
				{
					printf("\033[F");
					printf("\033[F\033[K");
					printf("\033[F\033[K");
					sigUser1 = 0;
					read(a2iFdPipe[0], &buffer[0], 1);
					iResult = GstPipe(&gif, buffer[0], gif_count);
					printf("\033[F\033[KPlaying gif nb : %i\n\r", gif);
					iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse[gif], &nb_frame);
					if (iResult < 0)
					{
						printf("error Get Nb Frames : %2x\n\r", iResult);
						goto cleanup;
					}
					printf("Valeur adresse (nb frame): %u\n\n\n\r", nb_frame);
				}

				for (size_t i = 0; i <= nb_frame && sigFlagStop != -1 && sigUser1 != 1; i++)
				{
					cmd_DisplayVideoFrame[6] = (i >> 8) & 0xFF;
					cmd_DisplayVideoFrame[7] = i & 0xFF;
					printf("\033[F\033[F\033[KDisplay Frame : %i\n\r", i);
					printf("=======================\n\r");
					iResult = SendCmd(EcranOled.m_iDescIO, cmd_DisplayVideoFrame, sizeof(cmd_DisplayVideoFrame));
					if (iResult != 0x06)
					{
						printf("error Display Video Frame : %2x\n\r", iResult);
						goto cleanup;
					}
					usleep(30000);
				}
			}

		//---------------- Setting down ----------------//
		cleanup:
			close(a2iFdPipe[0]);
			free(cmd_SetByteAdresse);
			printf("\n\033[F\033[KSetting down\n\r");
			iResult = ClearOled(EcranOled.m_iDescIO);
			if (iResult < 0)
			{
				printf("Error %d clearing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
			}
			iResult = change_baudrate(&EcranOled, &LiasonOled, BAUDRATE_9600, DEBUG_NONE);
			if (iResult < 0)
			{
			}

			tcsetattr(EcranOled.m_iDescIO, TCSANOW, &LiasonOled_Old); // restore old settings
			if (close(EcranOled.m_iDescIO) == -1)
			{
				printf("Error %d closing device %s: %s\n\r", errno, EcranOled.m_strFileIO, strerror(errno));
			}
			printf("Ecran closed\n\r");
			kill(pid, SIGTERM);
			w = waitpid(pid, &status, 0);
			if (w == -1)
			{
				perror("waitpid");
			}
			else
			{
			}
			printf("Pere end\n\r");
			printf("All process kill\n\r");
			fprintf(stdout, "Fin du programme\n\r");
		}
	}
	return (iResult);
}
