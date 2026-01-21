#include "main.h"

static long read_cmd_long(const char *cmd)
{
    long value = -1;
    FILE *pipe = popen(cmd, "r");
    if (pipe == NULL) {
        perror("popen");
        return -1;
    }
    if (fscanf(pipe, "%ld", &value) != 1) {
        fprintf(stderr, "Failed to read command output\n");
        value = -1;
    }
    if (pclose(pipe) == -1) {
        perror("pclose");
    }
    return value;
}

int main(void){

    int iResult = 0;
    
    FILE *file = fopen(TXT_FILE_PATH, "r"); //Remplacer le chemin d'acces par l'emplacement du fichier .txt
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    setvbuf(file, NULL, _IOFBF, 1 << 16);
    uint8_t buffer;
    unsigned int size_max = 0;

    if (fscanf(file, "%u", &size_max) != 1 || size_max == 0) {
        fprintf(stderr, "Invalid size in file\n");
        fclose(file);
        return -1;
    }
    printf("Nombre de ligne: %u\n", size_max);

    uint8_t (*montableau)[4] = calloc(size_max, sizeof *montableau);
    if (montableau == NULL) {
        perror("Error allocating table");
        fclose(file);
        return -1;
    }

    for (unsigned int i = 0; i < size_max; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            if (fscanf(file, "%2hhX", &buffer) != 1) {
                fprintf(stderr, "Unexpected end of data at row %u, col %u\n", i, j);
                free(montableau);
                fclose(file);
                return -1;
            }
            montableau[i][j] = buffer;
        }
    }
    for (unsigned int i = 0; i < size_max; i++) {
        printf("Entry %u: %02X %02X %02X %02X\n",
               i,
               montableau[i][0],
               montableau[i][1],
               montableau[i][2],
               montableau[i][3]);
    }

    uint32_t *addresses = calloc(size_max, sizeof *addresses);
    if (addresses == NULL) {
        perror("Error allocating addresses");
        free(montableau);
        fclose(file);
        return -1;
    }

    for (unsigned int i = 0; i < size_max; i++) {
        addresses[i] = ((uint32_t)montableau[i][0] << 24) |
                       ((uint32_t)montableau[i][1] << 16) |
                       ((uint32_t)montableau[i][2] << 8) |
                       (uint32_t)montableau[i][3];
        printf("Addr %u: 0x%08X\n", i, addresses[i]);
    }

    int addr3_int = (int)addresses[size_max - 1];
    printf("Adresse[3] en int: %d\n", addr3_int);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stat -c %%s %s", GIF_FILE_PATH); //Pareil ici changer en fonction de l'emplacement du fichier 

    long size = read_cmd_long(cmd);
    if (size >= 0) {
        printf("Taille gif.raw: %ld\n", size);
    }

    uint32_t next_address = (uint32_t)(addr3_int + size); 
    
    //aligne to next sector (512 bytes)
    uint32_t aligned_address = (next_address + 0x1FF) & ~0x1FF;
    printf("Next address aligned: 0x%08X\n", aligned_address);

    //add next aligned address to the table
    uint8_t byte0 = (aligned_address >> 24) & 0xFF;
    uint8_t byte1 = (aligned_address >> 16) & 0xFF;
    uint8_t byte2 = (aligned_address >> 8) & 0xFF;
    uint8_t byte3 = aligned_address & 0xFF; 

    //add next aligned address to the table
    montableau[size_max][0] = byte0;
    montableau[size_max][1] = byte1;
    montableau[size_max][2] = byte2;
    montableau[size_max][3] = byte3;

    //display the new entry
    printf("New Entry: %02X %02X %02X %02X\n",
           montableau[size_max][0],
           montableau[size_max][1],
           montableau[size_max][2],
           montableau[size_max][3]);    
    
    fclose(file);

    //write back to file
    file = fopen(TXT_FILE_PATH, "w");
    if (file == NULL) {
        perror("Error opening file for writing");   
    }

    fprintf(file, "%u\n", size_max + 1);
    for (unsigned int i = 0; i < size_max + 1; i++) {
        fprintf(file, "%02X%02X%02X%02X\n",
                montableau[i][0],
                montableau[i][1],
                montableau[i][2],
                montableau[i][3]);
    }
    fclose(file);

    free(addresses);
    free(montableau);
    
    return iResult;

}
