/* I2C definitions */
#if !defined __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <stddef.h>
#include <linux/i2c-dev.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>


// ADXL345 registers
#define ADXL345_REG_DEVID        0x00
#define ADXL345_REG_BW_RATE      0x2C
#define ADXL345_REG_POWER_CTL    0x2D
#define ADXL345_REG_INT_ENABLE   0x2E
#define ADXL345_REG_DATA_FORMAT  0x31
#define ADXL345_REG_DATAX0       0x32

// ADXL345 configuration values
#define ADXL345_BW_RATE_50HZ         0x09
#define ADXL345_POWER_CTL_MEASURE    0x08
#define ADXL345_INT_DATA_READY       0x80
#define ADXL345_DATA_FORMAT_4G       0x01


struct TCharDevice {
	const char 	m_strFileIO[60];
    const int m_I2cADDR;
	int		m_iDescIO;
    uint8_t m_u8ID;
};

int i2c_open(struct TCharDevice *__Dev);
int i2c_write_reg(struct TCharDevice *__Dev, uint8_t __Reg, uint8_t __Value);
int i2c_read_reg(struct TCharDevice *__Dev, uint8_t __Reg, uint8_t *__Value);
int i2c_read_block(struct TCharDevice *__Dev, uint8_t __StartReg, uint8_t *__Buf, size_t __Len);

#endif // #if !defined __I2C_H__
