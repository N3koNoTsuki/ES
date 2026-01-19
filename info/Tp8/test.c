#include "test.h"

#define BYTES_PER_LINE 16

int main(void)
{
    FILE *file;
    char filename[256];
    unsigned char buffer[BYTES_PER_LINE];
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
    uint16_t frame = 123;
    uint8_t cmd_SetByteAdresse[NBGIF][6] = {{0xFF, 0xB9, 0x02, 0x00, 0x00, 0x00},  // GIF 1
                                            {0xFF, 0xB9, 0x02, 0x04, 0x02, 0x00},  // GIF 2
                                            {0xFF, 0xB9, 0x02, 0x10, 0x04, 0x00},
                                            {0xFF, 0xB9, 0x02, 0x16, 0x82, 0x00}}; // GIF 3
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

    iResult = SendCmd(EcranOled.m_iDescIO, cmd_SetByteAdresse[3], 6);
    if (iResult != 0x06)
    {
        printf("error Set Byte Adresse : %2x\n\r", iResult);
        return -1;
    }

    iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse[3], &frame);
    if (iResult < 0)
    {
        printf("error Get Nb Frames : %2x\n\r", iResult);
        goto cleanup;
    }
    printf("Valeur adresse (nb frame): %u\n\r", frame);



    printf("Entrez le nom du fichier a lire : ");
    if (scanf("%255s", filename) != 1)
    {
        fprintf(stderr, "Erreur : nom de fichier invalide.\n");
        return 1;
    }

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        return 1;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        size_t i;

        printf("%08zx: ", offset);

        for (i = 0; i < bytes_read; i++)
        {
            printf("%02x ", buffer[i]);
            if (i == 7)
            {
                printf(" ");
            }
        }
        for (; i < BYTES_PER_LINE; i++)
        {
            printf("   ");
            if (i == 7)
            {
                printf(" ");
            }
        }
        printf("\n");

        i = 0;
        while (i + 1 < bytes_read)
        {
            Write_Word_media(&EcranOled, buffer[i], buffer[i + 1]);
            i += 2;
        }
        if (i < bytes_read)
        {
            Write_Word_media(&EcranOled, buffer[i], 0x00);
        }

        break;
    }


    iResult = GetNbFrames(&EcranOled, cmd_SetByteAdresse[3], &frame);
    if (iResult < 0)
    {
        printf("error Get Nb Frames : %2x\n\r", iResult);
        goto cleanup;
    }
    printf("Valeur adresse (nb frame) post ecriture: %u\n\r", frame);



    if (ferror(file))
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier.\n");
        fclose(file);
        return 1;
    }
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
    printf("Ecran closed\n\r");
    fprintf(stdout, "Fin du programme\n\r");

    fclose(file);
    return 0;
}
