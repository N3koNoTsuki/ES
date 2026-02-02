/* $Id: I2C.c, v 1.0 05/11/2017 11:51:52  $
 * linux : Upload/affiche un GIF sur l'OLED via RS-232
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 */
#include "I2C.h"

volatile sig_atomic_t sigFlagStop = 0;

void gestionnaire_signal(int iNumSignal);


static int16_t adxl345_sign_extend_10bit(uint16_t raw)
{
    raw &= 0x03FF;
    if (raw & 0x0200)
    {
        return (int16_t)(raw | 0xFC00);
    }
    return (int16_t)raw;
}

void gestionnaire_signal(int iNumSignal)
{
	if (iNumSignal == SIGINT || iNumSignal == SIGTERM)
	{
		sigFlagStop = -1;
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

    //---------------- Drivers ----------------//
    struct TCharDevice AccI2cDriver = {
        .m_strFileIO = "/dev/i2c-0",
        .m_I2cADDR = 0x53,
        .m_iDescIO = -1,
        .m_u8ID = ADXL345_REG_DEVID,
    };

    //---------------- Declaration ----------------//
    int iResult = -1;
    //---------------- Ouverture Drivers ----------------//

    iResult = i2c_open(&AccI2cDriver);
    if (iResult < 0)
    {
        perror("i2c_open :");
        goto cleanup;
    }

    uint8_t devid = 0;
    iResult = i2c_read_reg(&AccI2cDriver, AccI2cDriver.m_u8ID, &devid);
    if (iResult < 0)
    {
        perror("read devid :");
        goto cleanup;
    }
    printf("ADXL345 DEVID: 0x%02X\n\r", devid);

    // 50 Hz output data rate
    iResult = i2c_write_reg(&AccI2cDriver, ADXL345_REG_BW_RATE, ADXL345_BW_RATE_50HZ);
    if (iResult < 0)
    {
        perror("write BW_RATE :");
        goto cleanup;
    }

    // 10-bit, right-justified (FULL_RES=0, JUSTIFY=0) and +/-4 g range
    iResult = i2c_write_reg(&AccI2cDriver, ADXL345_REG_DATA_FORMAT, ADXL345_DATA_FORMAT_4G);
    if (iResult < 0)
    {
        perror("write DATA_FORMAT :");
        goto cleanup;
    }

    // Enable DATA_READY interrupt
    iResult = i2c_write_reg(&AccI2cDriver, ADXL345_REG_INT_ENABLE, ADXL345_INT_DATA_READY);
    if (iResult < 0)
    {
        perror("write INT_ENABLE :");
        goto cleanup;
    }

    // Measurement mode
    iResult = i2c_write_reg(&AccI2cDriver, ADXL345_REG_POWER_CTL, ADXL345_POWER_CTL_MEASURE);
    if (iResult < 0)
    {
        perror("write POWER_CTL :");
        goto cleanup;
    }

    // Read and print one sample (10-bit right-justified)
    uint8_t data[6] = {0};
    iResult = i2c_read_block(&AccI2cDriver, ADXL345_REG_DATAX0, data, sizeof(data));
    if (iResult < 0)
    {
        perror("read DATA :");
        goto cleanup;
    }

    int16_t x = adxl345_sign_extend_10bit((uint16_t)(data[1] << 8) | data[0]);
    int16_t y = adxl345_sign_extend_10bit((uint16_t)(data[3] << 8) | data[2]);
    int16_t z = adxl345_sign_extend_10bit((uint16_t)(data[5] << 8) | data[4]);

    printf("Accel raw (10-bit): X=%d Y=%d Z=%d\n\r", x, y, z);

    cleanup :
    if (AccI2cDriver.m_iDescIO >= 0)
    {
        close(AccI2cDriver.m_iDescIO);
    }

    return 0;
}
