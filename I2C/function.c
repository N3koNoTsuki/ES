#include "I2C.h"

int i2c_open(struct TCharDevice *__Dev)
{
    __Dev->m_iDescIO = open(__Dev->m_strFileIO, O_RDWR);
    if (__Dev->m_iDescIO < 0)
    {
        return -1;
    }

    if (ioctl(__Dev->m_iDescIO, I2C_SLAVE, __Dev->m_I2cADDR) < 0)
    {
        return -1;
    }

    return 0;
}

int i2c_write_reg(struct TCharDevice *__Dev, uint8_t __Reg, uint8_t __Value)
{
    uint8_t _Buf[2] = {__Reg, __Value};
    ssize_t _Written = write(__Dev->m_iDescIO, _Buf, sizeof(_Buf));
    if (_Written != (ssize_t)sizeof(_Buf))
    {
        return -1;
    }
    return 0;
}

int i2c_read_reg(struct TCharDevice *__Dev, uint8_t __Reg, uint8_t *__Value)
{
    return i2c_read_block(__Dev, __Reg, __Value, 1);
}

int i2c_read_block(struct TCharDevice *__Dev, uint8_t __StartReg, uint8_t *__Buf, size_t __Len)
{
    ssize_t _Written = write(__Dev->m_iDescIO, &__StartReg, 1);
    if (_Written != 1)
    {
        return -1;
    }

    ssize_t _Read_count = read(__Dev->m_iDescIO, __Buf, __Len);
    if (_Read_count != (ssize_t)__Len)
    {
        return -1;
    }

    return 0;
}

