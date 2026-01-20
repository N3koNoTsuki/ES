/* $Id: ProjetVF.c, v 1.0 05/11/2017 11:51:52  $
 * linux : Upload/affiche un GIF sur l'OLED via RS-232
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 */
#include "ProjetVF.h"

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


int main(void)
{
    // Clear console at startup
    printf("\033[2J\033[HDebut du programme\n\r");
	fflush(stdout);

    //---------------- Gestion des signaux ----------------//
    signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
    signal(SIGALRM, gestionnaire_signal);
	signal(SIGUSR1, gestionnaire_signal);


    FILE *file;
    char filename[256];
    char raw_filename[512];
    unsigned char buffer[BYTES_PER_SECTOR];
    size_t offset = 0;
    size_t bytes_read;

    //---------------- Drivers ----------------//
    struct TCharDevice EcranOled = {
        .m_strFileIO = "/dev/ttyAL0",
        .m_iDescIO = -1,
    };

    //---------------- Liaison serie RS-232 ----------------//
    struct termios LiasonOled, LiasonOled_Old = {.c_iflag = 0, .c_oflag = 0, .c_cflag = 0, .c_lflag = 0};

    //---------------- Reservation ----------------//

    int iResult = -1;
    uint16_t err = 0;
    uint16_t frame;
    uint8_t NoGif = 0;
    const uint8_t cmd_FlushMedia[2] = {0xFF, 0xB2};
    uint8_t cmd_DisplayVideoFrame[8] = {0xFF, 0xBA, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00};
    uint8_t (*cmd_SetByteAdresse)[6] = NULL;
    size_t gif_count = 0;
    uint8_t cmd_SetByteAdresse_Free[6] = {0};
    // Charge les adresses GIF + l'adresse libre depuis le fichier.
    if (LoadGifAddresses(GIF_ADDRESS_PATH, &cmd_SetByteAdresse, &gif_count) != 0)
    {
        fprintf(stderr, "Erreur lecture adresses: %s\n", GIF_ADDRESS_PATH);
        return -1;
    }
    if (gif_count == 0)
    {
        fprintf(stderr, "Aucune adresse GIF valide dans %s\n", GIF_ADDRESS_PATH);
        free(cmd_SetByteAdresse);
        return -1;
    }
    if (LoadGifFreeAddress(GIF_ADDRESS_PATH, cmd_SetByteAdresse_Free) != 0)
    {
        fprintf(stderr, "Erreur lecture adresse libre: %s\n", GIF_ADDRESS_PATH);
        free(cmd_SetByteAdresse);
        return -1;
    }

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

    iResult = init_media(EcranOled.m_iDescIO, &err);
    if (iResult < 0)
    {
        printf("%i\n\r", iResult);
        printf("error init Media : %2x\n\r", err);
        goto cleanup;
    }
    printf("Initialisation media ok\n\r");


    printf("Entrez le nom du fichier a lire : ");
    if (scanf("%255s", filename) != 1)
    {
        fprintf(stderr, "Erreur : nom de fichier invalide.\n");
        return -1;
    }

    if (filename[0] == 'c')
    {
        /* code */
        NoGif = 1;
    }
    
    if (NoGif == 0)
    {
        // Positionne l'adresse d'ecriture sur le prochain bloc libre.
        iResult = SendCmd(EcranOled.m_iDescIO, cmd_SetByteAdresse_Free, 6);
        if (iResult != 0x06)
        {
            printf("error Set Byte Adresse : %2x\n\r", iResult);
            goto cleanup;
        }

        snprintf(raw_filename, sizeof(raw_filename), "%s.raw", filename);
    
        if (build_raw_with_header(filename, raw_filename) != 0)
        {
            fprintf(stderr, "Erreur creation fichier raw.\n");
            return 1;
        }
        strncpy(filename, raw_filename, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    
    
    
        file = fopen(filename, "rb");
        if (file == NULL)
        {
            perror("Erreur lors de l'ouverture du fichier");
            return 1;
        }
        {
            struct stat st;
            size_t total_size = 0;
    
            if (fstat(fileno(file), &st) == 0 && st.st_size > 0)
            {
                total_size = (size_t)st.st_size;
            }
    
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0 && sigFlagStop != -1)
            {
                size_t line_offset = 0;
    
                while (line_offset < bytes_read && sigFlagStop != -1)
                {
                    size_t i;
                    size_t line_len = bytes_read - line_offset;
    
                    if (line_len > BYTES_PER_LINE)
                    {
                        line_len = BYTES_PER_LINE;
                    }
    
                    i = 0;
                    while (i + 1 < line_len && sigFlagStop != -1)
                    {
                        Write_Word_media(&EcranOled,
                                         buffer[line_offset + i],
                                         buffer[line_offset + i + 1]);
                        i += 2;
                    }
                    if (i < line_len)
                    {
                        Write_Word_media(&EcranOled,
                                         buffer[line_offset + i],
                                         0x00);
                    }
    
                    line_offset += line_len;
                }
    
                offset += bytes_read;
                print_progress(offset, total_size);
            }
        }


        iResult = SendCmd2RetV(EcranOled.m_iDescIO, cmd_FlushMedia, sizeof(cmd_FlushMedia), &err);
        if (iResult != 0)
        {
            /* code */
            printf("Erreur flush Media 0x%2X", err);
            goto cleanup;
        }
        if (AppendGifAddressFromRaw(GIF_ADDRESS_PATH, filename, NULL) != 0)
        {
            fprintf(stderr, "Erreur mise a jour adresses: %s\n", GIF_ADDRESS_PATH);
        }
        
    }
    
    if (sigFlagStop == -1)
    {
        /* code */
        goto cleanup;
    }
    
    
    if (NoGif == 0)
    {
        iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse_Free, &frame);
    }
    else
    {
        iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse[gif_count - 1], &frame);
    }
    if (iResult < 0)
    {
        printf("error Get Nb Frames : %2x\n\r", iResult);
        goto cleanup;
    }
    printf("\nValeur adresse (nb frame): %u\n\n\r", frame);

 
    for (size_t i = 0; i <= frame && sigFlagStop != -1 && sigUser1 != 1; i++)
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


    if (ferror(file))
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier.\n");
        fclose(file);
        return 1;
    }
    //---------------- Setting down ----------------//
 cleanup:
    printf("\n\033[F\033[KSetting down\n\r");
    free(cmd_SetByteAdresse);
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
    printf("Ecran closed\n\r");
    fprintf(stdout, "Fin du programme\n\r");


    if (sigFlagStop != -1)
    {
        /* code */
        char __Cmd[1024];
        int __Ret;

        snprintf(__Cmd, sizeof(__Cmd),
                    "./oled");

        __Ret = system(__Cmd);
        if (__Ret != 0)
        {
            return -1;
        }
    }
    

    fclose(file);
    return 0;
}
