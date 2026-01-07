#include "oled.h"

void clearcmd(uint8_t* cmd, size_t len){
    memset(cmd, 0, len);
}

int ClearOled(int __Fd){
    const uint8_t cmd[2] = {0xFF, 0xD7};
    uint8_t RetVal = 0;

	if (write(__Fd, cmd, sizeof(cmd)) < 0)
	{
		return -1;
	}
	if (read(__Fd, &RetVal, sizeof(RetVal)) < 0)
	{
		return -2;
	}

    return RetVal;
}

int SendCmd(int __Fd, const uint8_t * __buf, size_t len){

    uint8_t RetVal = 0;

    #ifdef DEBUG
    #endif

	if (write(__Fd, __buf, len) < 0)
	{
		#ifdef DEBUG
		perror("write ");
		#endif
		return -1; // propagate write error
	}
	if (read(__Fd, &RetVal, sizeof(RetVal)) < 0)
	{
		#ifdef DEBUG
		perror("read ");
		#endif
		return -2; // propagate read error
	}

	return RetVal;
}

void build_circle_cmd(uint8_t *cmd, uint8_t opcode, uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
	cmd[0] = 0xFF;
	cmd[1] = opcode;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (x >> 8) & 0xFF;
	cmd[3] = x & 0xFF;
	cmd[4] = (y >> 8) & 0xFF;
	cmd[5] = y & 0xFF;
	cmd[6] = (r >> 8) & 0xFF;
	cmd[7] = r & 0xFF;
	cmd[8] = (color >> 8) & 0xFF;
	cmd[9] = color & 0xFF;
}

void build_line_cmd(uint8_t *cmd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    cmd[0] = 0xFF;
	cmd[1] = 0xD2;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (x1 >> 8) & 0xFF;
	cmd[3] = x1 & 0xFF;
	cmd[4] = (y1 >> 8) & 0xFF;
	cmd[5] = y1 & 0xFF;
    cmd[6] = (x2 >> 8) & 0xFF;
	cmd[7] = x2 & 0xFF;
	cmd[8] = (y2 >> 8) & 0xFF;
	cmd[9] = y2 & 0xFF;
	cmd[10] = (color >> 8) & 0xFF;
	cmd[11] = color & 0xFF;
}

void Build_ChangeColor_cmd(uint8_t *cmd, uint16_t __Old, uint16_t __New)
{
    cmd[0] = 0xFF;
	cmd[1] = 0xBE;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (__Old >> 8) & 0xFF;
	cmd[3] = __Old & 0xFF;
	cmd[4] = (__New >> 8) & 0xFF;
	cmd[5] = __New & 0xFF;
}

static int read_exact(int fd, void *buf, size_t n)
{
    uint8_t *p = (uint8_t *)buf;
    size_t got = 0;

    while (got < n) {
        ssize_t r = read(fd, p + got, n - got);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) {
            return -2; 
        }
        got += (size_t)r;
    }
    return 0;
}

int init_media(int __Fd, uint16_t *__Ret)
{
    const uint8_t cmd[2] = {0xFF, 0xB1}; 

    if (write(__Fd, cmd, sizeof(cmd)) != (ssize_t)sizeof(cmd)) {
        return -1;
    }

    uint8_t ack = 0;
    if (read_exact(__Fd, &ack, 1) != 0) {
        return -2;
    }

    uint8_t b[2] = {0, 0};
    if (read_exact(__Fd, b, 2) != 0) {
        return -3;
    }

    uint16_t value = (uint16_t)((b[0] << 8) | b[1]); 
    *__Ret = value;

    if (ack != 0x06) {   
		printf("%02X\n\r", ack);     
        return -4;
    }
    if (value == 0) {           
        return -5;
    }

    return 0; 
}

int change_baudrate(struct TCharDevice device, struct termios* TermConfig, enum BaudRate baudRate, enum DebugLevel debugLevel){
    int iResult = 0;
    int BaudRateTermios = 0;
    u_int8_t ack = 0;
    u_int8_t BaudRateCmd[4] = {0x00,0x0B,0x00,0x00};

    // Set baudrate command bytes
    switch (baudRate)
    {
    case BAUDRATE_115200:
        BaudRateCmd[2] = 0x00;
        BaudRateCmd[3] = 0x19;
        BaudRateTermios = B115200;
        break;
    case BAUDRATE_9600:
        BaudRateCmd[2] = 0x01;
        BaudRateCmd[3] = 0x38;
        BaudRateTermios = B9600;
        break;
    default:
        if (debugLevel >= DEBUG_ERROR)
        {
            printf("Unknown baudrate %d\n", baudRate);
        }
        return -1;
        break;
    }

    // Send baudrate command to device

    iResult = write(device.m_iDescIO, BaudRateCmd, sizeof(BaudRateCmd));
    if (iResult == -1 && debugLevel >= DEBUG_ERROR)
    {
        printf("Error %d writing to device %s: %s\n", errno, device.m_strFileIO, strerror(errno));
    }
    usleep(100000); // wait 100 ms
    if (debugLevel >= DEBUG_INFO)
    {
        printf("Setting terminal %s speed to %i bps...\n", device.m_strFileIO, baudRate);
    }
    
    
    // Change terminal speed
   if(cfsetspeed(TermConfig, BaudRateTermios) == -1)
   {
        if (debugLevel >= DEBUG_ERROR)
        {printf("Error %d setting device %s speed: %s\n", errno, device.m_strFileIO, strerror(errno));}
        close(device.m_iDescIO);
        return -1;
   }
   if (debugLevel >= DEBUG_INFO)
   {
		printf("Applying new attributes to terminal %s...\n", device.m_strFileIO);
   }
   

   if(tcsetattr(device.m_iDescIO, TCSANOW, TermConfig) == -1)
   {
        if (debugLevel >= DEBUG_ERROR)
        {printf("Error %d setting device %s attributes: %s\n", errno, device.m_strFileIO, strerror(errno));}

        tcsetattr(device.m_iDescIO, TCSANOW, TermConfig); // restore old settings
        close(device.m_iDescIO);
        return -1;
   }


   // Read ack from device
   iResult = read(device.m_iDescIO, &ack, sizeof(ack));
   if (iResult == -1 && debugLevel >= DEBUG_ERROR)
    {
       printf("Error %d reading from device %s: %s\n", errno, device.m_strFileIO, strerror(errno));
    }
   else if (ack == 0x06 && debugLevel >= DEBUG_INFO)
    {
            printf("Ack received from device %s\n", device.m_strFileIO);
   }else if (debugLevel >= DEBUG_INFO)
   {
            printf("Bad ack received from device %s: 0x%02X\n", device.m_strFileIO, ack);
   }


   // Verify new speed of the terminal
   if (debugLevel >= DEBUG_INFO)
   {
    printf("Verifying new speed on device %s...\n", device.m_strFileIO);
   }
   
   if(tcgetattr(device.m_iDescIO, TermConfig) == -1){
        if (debugLevel >= DEBUG_ERROR)
        {
            printf("Error %d getting device %s attributes: %s\n", errno, device.m_strFileIO, strerror(errno));
        }
        
        close(device.m_iDescIO);
        return -1;

   }

   if (cfgetospeed(TermConfig) == BaudRateTermios && cfgetispeed(TermConfig) == BaudRateTermios)
   {
        if (debugLevel >= DEBUG_INFO)
        {
            printf("baudrate changed to %i\n", baudRate);
        }
   }

    return iResult;
}