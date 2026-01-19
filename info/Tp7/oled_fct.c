#include "oled.h"

/* read_exact
 * Role: read exactly __N bytes from __Fd into __Buf, handling EINTR.
 * Params:
 *  - __Fd: file descriptor to read from.
 *  - __Buf: output buffer where bytes are stored.
 *  - __N: number of bytes to read.
 * Return: 0 on success, -1 on read error, -2 on EOF before __N bytes.
 * Author: ChatGPT
 */
static int read_exact(int __Fd, void *__Buf, size_t __N)
{
    uint8_t *p = (uint8_t *)__Buf;
    size_t got = 0;

    while (got < __N) {
        ssize_t r = read(__Fd, p + got, __N - got);
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

/* ClearOled
 * Role: send clear-screen command to OLED and return device status.
 * Params:
 *  - __Fd: file descriptor of the OLED device.
 * Return: device status byte, or negative error code.
 * Author: Jules BELLEVRAS
 */
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

/* SendCmd
 * Role: send a command buffer and read a single-byte return value.
 * Params:
 *  - __Fd: file descriptor of the OLED device.
 *  - __Buf: command buffer to send.
 *  - __Len: number of bytes in command buffer.
 * Return: device status byte, or negative error code.
 * Author: Jules BELLEVRAS
 */
int SendCmd(int __Fd, const uint8_t * __Buf, size_t __Len){

    uint8_t RetVal = 0;

    tcflush(__Fd, TCIFLUSH);
    #ifdef DEBUG
    #endif

	if (write(__Fd, __Buf, __Len) < 0)
	{
		#ifdef DEBUG
		perror("write ");
		#endif
		return -1; 
	}
	if (read_exact(__Fd, &RetVal, sizeof(RetVal)) < 0)
	{
		#ifdef DEBUG
		perror("read ");
		#endif
		return -2; 
	}

	return RetVal;
}

/* SendCmd2RetV
 * Role: send command, read ACK and a 16-bit value returned by device.
 * Params:
 *  - __Fd: file descriptor of the OLED device.
 *  - __Buf: command buffer to send.
 *  - __Ret: output pointer to store 16-bit returned value.
 * Return: 0 on success, negative error code on failure/invalid data.
 * Author: Jules BELLEVRAS
 */
int SendCmd2RetV(int __Fd, const uint8_t * __Buf, uint16_t *__Ret)
{
    tcflush(__Fd, TCIFLUSH);

    if (write(__Fd, __Buf, sizeof(__Buf)) != (ssize_t)sizeof(__Buf)) {
        return -1;
    }
    usleep(500000);
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

/* change_baudrate
 * Role: send baudrate command to device and update terminal speed.
 * Params:
 *  - __Dev: device handle (fd + path).
 *  - __Term: termios settings to update.
 *  - __BaudRate: target baudrate enum.
 *  - __DbgLvl: debug verbosity level.
 * Return: write/read result or negative error on failure.
 * Author: Quentin Casagranda
 * Modified: Jules BELLEVRAS
 */
int change_baudrate(struct TCharDevice *__Dev, struct termios* __Term, enum BaudRate __BaudRate, enum DebugLevel __DbgLvl){
    int iResult = 0;
    int BaudRateTermios = 0;
    u_int8_t ack = 0;
    u_int8_t BaudRateCmd[4] = {0x00,0x0B,0x00,0x00};

    // Set baudrate command bytes
    switch (__BaudRate)
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
        if (__DbgLvl >= DEBUG_ERROR)
        {
            printf("Unknown baudrate %d\n", __BaudRate);
        }
        return -1;
        break;
    }

    // Send baudrate command to device
    tcflush(__Dev->m_iDescIO, TCIOFLUSH);

    iResult = write(__Dev->m_iDescIO, BaudRateCmd, sizeof(BaudRateCmd));
    if (iResult == -1 && __DbgLvl >= DEBUG_ERROR)
    {
        printf("Error %d writing to device %s: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
    }
    usleep(120000); // wait 100 ms
    if (__DbgLvl >= DEBUG_INFO)
    {
        printf("Setting terminal %s speed to %i bps...\n", __Dev->m_strFileIO, __BaudRate);
    }
    
    
    // Change terminal speed
   if(cfsetspeed(__Term, BaudRateTermios) == -1)
   {
        if (__DbgLvl >= DEBUG_ERROR)
        {printf("Error %d setting device %s speed: %s\n", errno, __Dev->m_strFileIO, strerror(errno));}
        close(__Dev->m_iDescIO);
        return -1;
   }
   if (__DbgLvl >= DEBUG_INFO)
   {
		printf("Applying new attributes to terminal %s...\n", __Dev->m_strFileIO);
   }
   

   if(tcsetattr(__Dev->m_iDescIO, TCSANOW, __Term) == -1)
   {
        if (__DbgLvl >= DEBUG_ERROR)
        {printf("Error %d setting device %s attributes: %s\n", errno, __Dev->m_strFileIO, strerror(errno));}

        tcsetattr(__Dev->m_iDescIO, TCSANOW, __Term); // restore old settings
        close(__Dev->m_iDescIO);
        return -1;
   }


   // Read ack from device
   iResult = read(__Dev->m_iDescIO, &ack, 1);
   if (iResult == -1 && __DbgLvl >= DEBUG_ERROR)
    {
       printf("Error %d reading from device %s: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
    }
   else if (ack == 0x06 && __DbgLvl >= DEBUG_INFO)
    {
            printf("Ack received from device %s\n", __Dev->m_strFileIO);
   }else if (__DbgLvl >= DEBUG_INFO)
   {
            printf("Bad ack received from device %s: 0x%02X\n", __Dev->m_strFileIO, ack);
   }


   // Verify new speed of the terminal
   if (__DbgLvl >= DEBUG_INFO)
   {
    printf("Verifying new speed on device %s...\n", __Dev->m_strFileIO);
   }
   
   if(tcgetattr(__Dev->m_iDescIO, __Term) == -1){
        if (__DbgLvl >= DEBUG_ERROR)
        {
            printf("Error %d getting device %s attributes: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
        }
        
        close(__Dev->m_iDescIO);
        return -1;

   }

   if (cfgetospeed(__Term) == BaudRateTermios && cfgetispeed(__Term) == BaudRateTermios)
   {
        if (__DbgLvl >= DEBUG_INFO)
        {
            printf("baudrate changed to %i\n", __BaudRate);
        }
   }

    return iResult;
}

/* init_liaison
 * Role: initialize serial link to OLED, set raw mode and switch baudrate.
 * Params:
 *  - __Dev: device handle (fd + path).
 *  - __Init: termios buffer to configure.
 *  - __Old: termios buffer for saving previous settings.
 *  - __lenInit: size of termios struct for memcpy.
 * Return: 0 or positive on success, negative error code on failure.
 * Author: Jules BELLEVRAS
 */
int init_liaison(struct TCharDevice *__Dev, struct termios *__Init, struct termios *__Old, size_t __lenInit){
    
    int __RET = -1;

    if (tcgetattr(__Dev->m_iDescIO, __Init) < 0)
	{
		perror("tcgetattr");
		close(__Dev->m_iDescIO);
		return -1;
	}
	memcpy(__Old, __Init, __lenInit);
	__Old = __Init; // Buffer pour enregistrer les params par defauts
	cfmakeraw(__Init);
	__Init->c_cflag |= (CLOCAL | CREAD); // ignore modem control lines and enable receiver
	__Init->c_cflag &= ~PARENB;			// no parity
	__Init->c_cflag &= ~CSTOPB;			// 1 stop bit
	__Init->c_cflag &= ~CSIZE;			// clear data bits setting
	__Init->c_cflag |= CS8;				// 8 data bits
	__Init->c_cc[VTIME] = 1;
	__Init->c_cc[VMIN] = 0;
	__RET = cfsetspeed(__Init, B9600);
	if (__RET < 0)
	{
		perror("cfsetspeed");
		close(__Dev->m_iDescIO);
		return -2;
	}
	__RET = tcsetattr(__Dev->m_iDescIO, TCSANOW, __Init);
	if (__RET < 0)
	{
		tcsetattr(__Dev->m_iDescIO, TCSANOW, __Old);
		perror("tcsetattr");
		close(__Dev->m_iDescIO);
		return -3;
	}
	if (fcntl(__Dev->m_iDescIO, F_SETFL, 0))
	{
		printf("Error %d setting device %s flags: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
		tcsetattr(__Dev->m_iDescIO, TCSANOW, __Old); // restore old settings
		close(__Dev->m_iDescIO);
		return -4;
	}
	sleep(3);
    __RET = change_baudrate(__Dev, __Init, BAUDRATE_115200, DEBUG_NONE);
	if (__RET < 0)
	{
		return -5;
	}
    return __RET;
}

/* init_media
 * Role: request media init/info and read ACK + 16-bit value.
 * Params:
 *  - __Fd: file descriptor of the OLED device.
 *  - __Ret: output pointer to store 16-bit returned value.
 * Return: 0 on success, negative error code on failure/invalid data.
 * Author: Jules BELLEVRAS
 */
int init_media(int __Fd, uint16_t *__Ret)
{
    const uint8_t cmd[2] = {0xFF, 0xB1}; 

    tcflush(__Fd, TCIFLUSH);

    if (write(__Fd, cmd, sizeof(cmd)) != (ssize_t)sizeof(cmd)) {
        return -1;
    }
    usleep(500000);
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


/* config_gpio_button
 * Role: open GPIO chip and configure two lines as event sources.
 * Params:
 *  - __Fds: GPIO device struct to initialize.
 *  - __EventFlgs: edge trigger flags for the events.
 *  - __DbgLvl: verbosity for diagnostics.
 * Return: 0 on success, negative error code on failure.
 * Author: Quentin Casagranda
 * Modified: Jules BELLEVRAS
 */
int config_gpio_button(struct TDeviceCharGpIO* __Fds, unsigned int __EventFlgs, enum DebugLevel __DbgLvl){

    int iResult = 0;
    __Fds->m_DevChar.m_iDescIO = open(__Fds->m_DevChar.m_strFileIO, O_RDWR);
    if (__Fds->m_DevChar.m_iDescIO < 0)
    {
        perror("open() button1\n\r");
        return -1;
    }
    ////////////////	Récuperation Infos GPIO Bouttons côtés	//////////////
    memset(&__Fds->m_chipinfo,0, sizeof(__Fds->m_chipinfo));
    iResult = ioctl(__Fds->m_DevChar.m_iDescIO, GPIO_GET_CHIPINFO_IOCTL,&__Fds->m_chipinfo);
    if (iResult < 0 && __DbgLvl >= DEBUG_ERROR)
    {
        printf( "Erreur recuperation info Boutton D/G : ");
        printf("\n");
    }else if (__DbgLvl >= DEBUG_VERBOSE) {
        printf("//////////////////////	GPIO BOUTTON D/G	////////////////////////\n");
        printf("name : %s\n" ,__Fds->m_chipinfo.name);
        printf( "label : %s\n",__Fds->m_chipinfo.label);
        printf( "number of gpio : %i\n",__Fds->m_chipinfo.lines);
        printf("////////////////////////////////////////////////////////////////////\n");
    }
    

    /////////////////////	Config Boutton Droit	////////////////////
    __Fds->m_event_request[0].lineoffset = 0;
    __Fds->m_event_request[0].handleflags = GPIOHANDLE_REQUEST_INPUT;
    __Fds->m_event_request[0].eventflags = __EventFlgs;

    iResult = ioctl(__Fds->m_DevChar.m_iDescIO, GPIO_GET_LINEEVENT_IOCTL,&__Fds->m_event_request[0]);
    if (iResult < 0 && __DbgLvl >= DEBUG_ERROR)
    {
        perror("erreur configuration comportement Boutton Droit : ");
        printf("\n");
    }

    /////////////////////	Config Boutton Gauche	////////////////////
    __Fds->m_event_request[1].lineoffset = 1;
    __Fds->m_event_request[1].handleflags = GPIOHANDLE_REQUEST_INPUT;
    __Fds->m_event_request[1].eventflags = __EventFlgs;

    iResult = ioctl(__Fds->m_DevChar.m_iDescIO, GPIO_GET_LINEEVENT_IOCTL,&__Fds->m_event_request[1]);
    if (iResult < 0 && __DbgLvl >= DEBUG_ERROR)
    {
        perror("erreur configuration comportement Boutton Gauche: ");
        printf("\n");
    }

    ///////////////	Config Poll Bouttons côtés	//////////////////
    __Fds->m_pollfd[0].fd = __Fds->m_event_request[0].fd;
    __Fds->m_pollfd[0].events = POLLIN;
    __Fds->m_pollfd[1].fd = __Fds->m_event_request[1].fd;
    __Fds->m_pollfd[1].events = POLLIN;

    return iResult;
}

/* button_press
 * Role: poll a GPIO event line and report a button press on falling edge.
 * Params:
 *  - __Fds: GPIO device handle struct with pollfd/event buffers.
 *  - __Idx: index of the GPIO line to check.
 * Return: 1 on falling edge, 2 on other event, 0 on timeout, -1 on error.
 * Author: Jules BELLEVRAS
 */
int button_press(struct TDeviceCharGpIO *__Fds, int __Idx){
    int __Ret = -1;
    __Ret = poll(&__Fds->m_pollfd[__Idx], 1, 10);
    if (__Ret < 0)
    { 
        return -1;
    }
    else if (__Ret > 0 && (__Fds->m_pollfd[__Idx].revents & POLLIN))
    {
        __Ret = 2;
        read(__Fds->m_pollfd[__Idx].fd, &__Fds->m_event_data[__Idx], sizeof(__Fds->m_event_data[__Idx]));
        if (__Fds->m_event_data[__Idx].id == GPIOEVENT_EVENT_FALLING_EDGE)
        {
            return 1;
        }
    }
    
    return __Ret;
}

/* check_all_buttons
 * Role: check four button lines and return which one was pressed.
 * Params:
 *  - gpioButton1: GPIO device with two lines (indexes 0,1).
 *  - gpioButton2: GPIO device with two lines (indexes 0,1).
 * Return: 1..4 for a press, -1 if none pressed or on error.
 * Author: Jules BELLEVRAS
 */
int check_all_buttons(struct TDeviceCharGpIO* __Fds1, struct TDeviceCharGpIO* __Fds2){
    int Ret1 = button_press(__Fds1, 0);
    int Ret2 = button_press(__Fds1, 1);
    int Ret3 = button_press(__Fds2, 0);
    int Ret4 = button_press(__Fds2, 1);
    if (Ret1 == 1)
    {
        return 1; 
    }
    if (Ret2 == 1)
    {
        return 2; 
    }
    if (Ret3 == 1)
    {
        return 3; 
    }
    if (Ret4 == 1)
    {
        return 4; 
    }

    return -1; 
}

/* GstPipe
 * Role: update GIF index based on button code.
 * Params:
 *  - __Gif: current GIF index.
 *  - __Idx: button code ('1'..'4').
 * Return: action code 1..4 when handled, -1 otherwise.
 * Author: Jules BELLEVRAS
 */
int GstPipe(int *__Gif, char __Idx ){
    int __Ret = -1;
    if (__Idx == 3)
    {
        *__Gif = (*__Gif + 1) % NBGIF;
        __Ret = 1;
    }
    else if (__Idx == 4)
    {
        *__Gif = (*__Gif - 1) % NBGIF;
        if (*__Gif < 0)
        {
            *__Gif = NBGIF - 1;
        }
        
        __Ret = 2;
    }
    else if (__Idx == 2)
    {
        *__Gif = 0;
        __Ret = 3;
    }
    else if (__Idx == 1)
    {
        *__Gif = NBGIF - 1;
        __Ret = 4;
    }
    return __Ret;
}

/* GetNbFrames
 * Role: set byte address, read number of frames, then restore address.
 * Params:
 *  - __Fds: OLED device handle.
 *  - __CmdSetByte: command buffer to set byte address (updated in place)(start at adresse + 0x07 which is the number of frames).
 *  - __NbFrames: output pointer to store the frame count.
 * Return: 0x06 on success, negative error code on failure.
 * Author: Jules BELLEVRAS
 */
int GetNbFrames(struct TCharDevice *__Fds, uint8_t __CmdSetByte[], uint16_t* __NbFrames){

    int __RET = -1;
    const uint8_t CmdReadByte[2] = {0xFF, 0xB7};

    //On se place au byte contenant le nombre de frames
    __CmdSetByte[5] = 0x07;

    __RET = SendCmd(__Fds->m_iDescIO, __CmdSetByte, 6);
    if (__RET != 0x06)
    {
        printf("error Set Byte Adresse : %2x\n\r", __RET);
        return -1;
    }

    __RET = SendCmd2RetV(__Fds->m_iDescIO, CmdReadByte, __NbFrames);
    if (__RET < 0)
    {
        printf("error Read Byte : %2x\n\r", __RET);
        return -2;
    }
   
    
    //On se remet a la bonne adresse pour la video.
    __CmdSetByte[5] = 0x00;
    
    __RET = SendCmd(__Fds->m_iDescIO, __CmdSetByte, 6);
    if (__RET != 0x06)
    {
        printf("error Set Byte Adresse def : %2x\n\r", __RET);
        return -3;
    }
    return __RET;
}
