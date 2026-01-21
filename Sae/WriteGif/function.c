#include "WriteGif.h"

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
    uint8_t *__Ptr = (uint8_t *)__Buf;
    size_t __Got = 0;

    while (__Got < __N) {
        ssize_t __Read = read(__Fd, __Ptr + __Got, __N - __Got);
        if (__Read < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (__Read == 0) {
            return -2; 
        }
        __Got += (size_t)__Read;
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
    const uint8_t __Cmd[2] = {0xFF, 0xD7};
    uint8_t __RetVal = 0;

	if (write(__Fd, __Cmd, sizeof(__Cmd)) < 0)
	{
		return -1;
	}
	if (read(__Fd, &__RetVal, sizeof(__RetVal)) < 0)
	{
		return -2;
	}

    return __RetVal;
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

    uint8_t __RetVal = 0;

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
	if (read_exact(__Fd, &__RetVal, sizeof(__RetVal)) < 0)
	{
		#ifdef DEBUG
		perror("read ");
		#endif
		return -2; 
	}

	return __RetVal;
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
int SendCmd2RetV(int __Fd, const uint8_t * __Buf, size_t __Len, uint16_t *__Ret)
{
    tcflush(__Fd, TCIFLUSH);

    if (write(__Fd, __Buf, __Len) != (ssize_t)__Len) {
        return -1;
    }
    uint8_t __Ack = 0;
    if (read_exact(__Fd, &__Ack, 1) != 0) {
        return -2;
    }

    uint8_t __BufRet[2] = {0, 0};
    if (read_exact(__Fd, __BufRet, 2) != 0) {
        return -3;
    }

    uint16_t __Value = (uint16_t)((__BufRet[0] << 8) | __BufRet[1]); 
    *__Ret = __Value;

    if (__Ack != 0x06) {   
		printf("%02X\n\r", __Ack);     
        return -4;
    }
    if (__Value == 0) {           
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
    int __Result = 0;
    int __BaudRateTermios = 0;
    u_int8_t __Ack = 0;
    u_int8_t __BaudRateCmd[4] = {0x00,0x0B,0x00,0x00};

    // Set baudrate command bytes
    switch (__BaudRate)
    {
    case BAUDRATE_115200:
        __BaudRateCmd[2] = 0x00;
        __BaudRateCmd[3] = 0x19;
        __BaudRateTermios = B115200;
        break;
    case BAUDRATE_9600:
        __BaudRateCmd[2] = 0x01;
        __BaudRateCmd[3] = 0x38;
        __BaudRateTermios = B9600;
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

    __Result = write(__Dev->m_iDescIO, __BaudRateCmd, sizeof(__BaudRateCmd));
    if (__Result == -1 && __DbgLvl >= DEBUG_ERROR)
    {
        printf("Error %d writing to device %s: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
    }
    usleep(120000); // wait 100 ms
    if (__DbgLvl >= DEBUG_INFO)
    {
        printf("Setting terminal %s speed to %i bps...\n", __Dev->m_strFileIO, __BaudRate);
    }
    
    
    // Change terminal speed
   if(cfsetspeed(__Term, __BaudRateTermios) == -1)
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
   __Result = read(__Dev->m_iDescIO, &__Ack, 1);
   if (__Result == -1 && __DbgLvl >= DEBUG_ERROR)
    {
       printf("Error %d reading from device %s: %s\n", errno, __Dev->m_strFileIO, strerror(errno));
    }
   else if (__Ack == 0x06 && __DbgLvl >= DEBUG_INFO)
    {
            printf("Ack received from device %s\n", __Dev->m_strFileIO);
   }else if (__DbgLvl >= DEBUG_INFO)
   {
            printf("Bad ack received from device %s: 0x%02X\n", __Dev->m_strFileIO, __Ack);
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

   if (cfgetospeed(__Term) == __BaudRateTermios && cfgetispeed(__Term) == __BaudRateTermios)
   {
        if (__DbgLvl >= DEBUG_INFO)
        {
            printf("baudrate changed to %i\n", __BaudRate);
        }
   }

    return __Result;
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
    const uint8_t __Cmd[2] = {0xFF, 0xB1}; 

    tcflush(__Fd, TCIFLUSH);

    if (write(__Fd, __Cmd, sizeof(__Cmd)) != (ssize_t)sizeof(__Cmd)) {
        return -1;
    }
    usleep(500000);
    uint8_t __Ack = 0;
    if (read_exact(__Fd, &__Ack, 1) != 0) {
        return -2;
    }

    uint8_t __BufRet[2] = {0, 0};
    if (read_exact(__Fd, __BufRet, 2) != 0) {
        return -3;
    }

    uint16_t __Value = (uint16_t)((__BufRet[0] << 8) | __BufRet[1]); 
    *__Ret = __Value;

    if (__Ack != 0x06) {   
		printf("%02X\n\r", __Ack);     
        return -4;
    }
    if (__Value == 0) {           
        return -5;
    }

    return 0; 
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
    const uint8_t __CmdReadByte[2] = {0xFF, 0xB7};

    //On se place au byte contenant le nombre de frames (MSB)
    __CmdSetByte[5] = 0x07;

    __RET = SendCmd(__Fds->m_iDescIO, __CmdSetByte, 6);
    if (__RET != 0x06)
    {
        printf("error Set Byte Adresse : %2x\n\r", __RET);
        return -1;
    }

    __RET = SendCmd2RetV(__Fds->m_iDescIO, __CmdReadByte, sizeof(__CmdReadByte), __NbFrames);
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

/* Write_Byte_media
 * Role: write a single data byte to the media using the write-word command.
 * Params:
 *  - __Fds: OLED device handle.
 *  - __Lsb: byte to write (LSB in 16-bit payload).
 * Return: device status or negative error code.
 * Author: Jules BELLEVRAS
 */
int Write_Byte_media(struct TCharDevice *__Fds, uint8_t __Lsb){

    uint8_t __Cmd[4] = {0xFF, 0xB4, 0x00, __Lsb};
    uint16_t __Ret = 0;
    SendCmd2RetV(__Fds->m_iDescIO, __Cmd, sizeof(__Cmd), &__Ret);
    return __Ret;
}

/* Write_Word_media
 * Role: write a 16-bit word to the media.
 * Params:
 *  - __Fds: OLED device handle.
 *  - __Msb: most significant byte.
 *  - __Lsb: least significant byte.
 * Return: device status or negative error code.
 * Author: Jules BELLEVRAS
 */
int Write_Word_media(struct TCharDevice *__Fds, uint8_t __Msb, uint8_t __Lsb){

    uint8_t __Cmd[4] = {0xFF, 0xB4, __Msb, __Lsb};
    uint16_t __Ret = 0;
    SendCmd2RetV(__Fds->m_iDescIO, __Cmd, sizeof(__Cmd), &__Ret);
    return __Ret;
}

/* print_progress
 * Role: display a simple progress bar for a byte transfer.
 * Params:
 *  - __Written: number of bytes written so far.
 *  - __Total: total number of bytes to write.
 * Return: none.
 * Author: Jules BELLEVRAS
 * Modified: ChatGPT
 */
void print_progress(size_t __Written, size_t __Total){
    const int __BarWidth = 30;
    int __Filled = 0;
    int __Percent = 0;

    if (__Total > 0)
    {
        __Percent = (int)((__Written * 100) / __Total);
        __Filled = (int)((__Written * __BarWidth) / __Total);
        if (__Filled > __BarWidth)
        {
            __Filled = __BarWidth;
        }
    }

    printf("\r[");
    for (int __Idx = 0; __Idx < __BarWidth; __Idx++)
    {
        printf(__Idx < __Filled ? "#" : "-");
    }
    printf("] %3d%%", __Percent);
    fflush(stdout);
}

/* LoadGifAddresses
 * Role: read GIF byte addresses from a file and build SetByte command buffers.
 * Params:
 *  - __Path: address file path.
 *  - __CmdSetByte: output command buffers (allocated).
 *  - __OutCount: output number of GIF entries (count-1).
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 * Modified: ChatGPT
 */
int LoadGifAddresses(const char *__Path, uint8_t (**__CmdSetByte)[6], size_t *__OutCount)
{
    FILE *__File = NULL;
    unsigned int __Count = 0;
    int __Ret = -1;

    if (__Path == NULL || __CmdSetByte == NULL || __OutCount == NULL) {
        return -1;
    }
    *__CmdSetByte = NULL;
    *__OutCount = 0;

    __File = fopen(__Path, "r");
    if (__File == NULL) {
        perror("fopen");
        return -1;
    }

    if (fscanf(__File, "%u", &__Count) != 1 || __Count == 0) {
        fprintf(stderr, "Invalid address count in %s\n", __Path);
        goto cleanup;
    }

    size_t __UseCount = (size_t)__Count - 1;
    uint8_t (*__Out)[6] = NULL;
    if (__UseCount > 0) {
        __Out = calloc(__UseCount, sizeof(*__Out));
        if (__Out == NULL) {
            perror("calloc");
            goto cleanup;
        }
    }

    for (unsigned int __I = 0; __I < __Count; __I++) {
        uint8_t __Entry[6];
        for (unsigned int __J = 0; __J < 6; __J++) {
            if (fscanf(__File, "%2hhX", &__Entry[__J]) != 1) {
                fprintf(stderr, "Invalid address data at entry %u\n", __I);
                free(__Out);
                goto cleanup;
            }
        }
        if (__I < __UseCount) {
            for (unsigned int __J = 0; __J < 6; __J++) {
                __Out[__I][__J] = __Entry[__J];
            }
        }
    }

    *__CmdSetByte = __Out;
    *__OutCount = __UseCount;
    __Ret = 0;

cleanup:
    fclose(__File);
    return __Ret;
}

/* EnsureGifAddressFile
 * Role: create a minimal gif_addresses.txt file if it does not exist.
 * Params:
 *  - __Path: address file path.
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 * Modified: ChatGPT
 */
int EnsureGifAddressFile(const char *__Path)
{
    FILE *__File = NULL;
    struct stat __St;
    const char *__Slash = NULL;

    if (__Path == NULL) {
        return -1;
    }

    __File = fopen(__Path, "r");
    if (__File != NULL) {
        fclose(__File);
        return 0;
    }
    if (errno != ENOENT) {
        perror("fopen");
        return -1;
    }

    __Slash = strrchr(__Path, '/');
    if (__Slash != NULL && __Slash != __Path) {
        char __Dir[512];
        size_t __Len = (size_t)(__Slash - __Path);
        if (__Len >= sizeof(__Dir)) {
            fprintf(stderr, "Path too long: %s\n", __Path);
            return -1;
        }
        memcpy(__Dir, __Path, __Len);
        __Dir[__Len] = '\0';
        if (stat(__Dir, &__St) != 0) {
            if (mkdir(__Dir, 0700) != 0 && errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
        }
    }

    __File = fopen(__Path, "w");
    if (__File == NULL) {
        perror("fopen");
        return -1;
    }

    if (fputs("1\nFFB902000000\n", __File) == EOF) {
        fclose(__File);
        return -1;
    }

    fclose(__File);
    return 0;
}

/* LoadGifFreeAddress
 * Role: read the last entry from the address file (next free address).
 * Params:
 *  - __Path: address file path.
 *  - __CmdSetByte: output command buffer (size 6).
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 */
int LoadGifFreeAddress(const char *__Path, uint8_t __CmdSetByte[6])
{
    FILE *__File = NULL;
    unsigned int __Count = 0;
    uint8_t __Entry[6] = {0};

    if (__Path == NULL || __CmdSetByte == NULL) {
        return -1;
    }

    __File = fopen(__Path, "r");
    if (__File == NULL) {
        perror("fopen");
        return -1;
    }

    if (fscanf(__File, "%u", &__Count) != 1 || __Count == 0) {
        fprintf(stderr, "Invalid address count in %s\n", __Path);
        fclose(__File);
        return -1;
    }

    for (unsigned int __I = 0; __I < __Count; __I++) {
        for (unsigned int __J = 0; __J < 6; __J++) {
            if (fscanf(__File, "%2hhX", &__Entry[__J]) != 1) {
                fprintf(stderr, "Invalid address data at entry %u\n", __I);
                fclose(__File);
                return -1;
            }
        }
    }

    fclose(__File);
    for (unsigned int __J = 0; __J < 6; __J++) {
        __CmdSetByte[__J] = __Entry[__J];
    }
    return 0;
}

/* AppendGifAddressFromRaw
 * Role: append the next aligned address to the address file based on raw file size.
 * Params:
 *  - __AddrPath: address file path.
 *  - __RawPath: raw file path used to compute the next address.
 *  - __OutAddress: optional output of the appended address.
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 */
int AppendGifAddressFromRaw(const char *__AddrPath, const char *__RawPath, uint32_t *__OutAddress)
{
    FILE *__File = NULL;
    unsigned int __Count = 0;
    uint8_t (*__Entries)[6] = NULL;
    struct stat __St;
    uint32_t __LastAddr = 0;
    uint32_t __NextAddr = 0;
    uint32_t __AlignedAddr = 0;

    if (__AddrPath == NULL || __RawPath == NULL) {
        return -1;
    }

    __File = fopen(__AddrPath, "r");
    if (__File == NULL) {
        perror("fopen");
        return -1;
    }

    if (fscanf(__File, "%u", &__Count) != 1 || __Count == 0) {
        fprintf(stderr, "Invalid address count in %s\n", __AddrPath);
        fclose(__File);
        return -1;
    }

    __Entries = calloc(__Count + 1, sizeof(*__Entries));
    if (__Entries == NULL) {
        perror("calloc");
        fclose(__File);
        return -1;
    }

    for (unsigned int __I = 0; __I < __Count; __I++) {
        for (unsigned int __J = 0; __J < 6; __J++) {
            if (fscanf(__File, "%2hhX", &__Entries[__I][__J]) != 1) {
                fprintf(stderr, "Invalid address data at entry %u\n", __I);
                free(__Entries);
                fclose(__File);
                return -1;
            }
        }
    }
    fclose(__File);

    __LastAddr = ((uint32_t)__Entries[__Count - 1][2] << 24) |
                 ((uint32_t)__Entries[__Count - 1][3] << 16) |
                 ((uint32_t)__Entries[__Count - 1][4] << 8) |
                 (uint32_t)__Entries[__Count - 1][5];

    if (stat(__RawPath, &__St) != 0 || __St.st_size < 0) {
        perror("stat");
        free(__Entries);
        return -1;
    }

    __NextAddr = __LastAddr + (uint32_t)__St.st_size;
    __AlignedAddr = (__NextAddr + 0x1FFu) & ~0x1FFu;

    __Entries[__Count][0] = 0xFF;
    __Entries[__Count][1] = 0xB9;
    __Entries[__Count][2] = (uint8_t)((__AlignedAddr >> 24) & 0xFF);
    __Entries[__Count][3] = (uint8_t)((__AlignedAddr >> 16) & 0xFF);
    __Entries[__Count][4] = (uint8_t)((__AlignedAddr >> 8) & 0xFF);
    __Entries[__Count][5] = (uint8_t)(__AlignedAddr & 0xFF);

    __File = fopen(__AddrPath, "w");
    if (__File == NULL) {
        perror("fopen");
        free(__Entries);
        return -1;
    }

    fprintf(__File, "%u\n", __Count + 1);
    for (unsigned int __I = 0; __I < __Count + 1; __I++) {
        fprintf(__File, "%02X%02X%02X%02X%02X%02X\n",
                __Entries[__I][0],
                __Entries[__I][1],
                __Entries[__I][2],
                __Entries[__I][3],
                __Entries[__I][4],
                __Entries[__I][5]);
    }
    fclose(__File);

    if (__OutAddress != NULL) {
        *__OutAddress = __AlignedAddr;
    }

    free(__Entries);
    return 0;
}

/* write_oled_header
 * Role: write an 8-byte OLED header to a file.
 * Params:
 *  - __Out: file stream to write into.
 *  - __Width: image width in pixels.
 *  - __Height: image height in pixels.
 *  - __Bpp: bits per pixel.
 *  - __Delay: frame delay (centiseconds).
 *  - __Frames: number of frames.
 * Return: 0 on success, -1 on write error.
 * Author: Jules BELLEVRAS
 * Modified: ChatGPT
 */
static int write_oled_header(FILE *__Out, uint16_t __Width, uint16_t __Height, uint8_t __Bpp, uint8_t __Delay, uint16_t __Frames)
{
    uint8_t __Header[8];

    __Header[0] = (uint8_t)((__Width >> 8) & 0xFF);
    __Header[1] = (uint8_t)(__Width & 0xFF);
    __Header[2] = (uint8_t)((__Height >> 8) & 0xFF);
    __Header[3] = (uint8_t)(__Height & 0xFF);
    __Header[4] = __Bpp;
    __Header[5] = __Delay;
    __Header[6] = (uint8_t)((__Frames >> 8) & 0xFF);
    __Header[7] = (uint8_t)(__Frames & 0xFF);

    return (fwrite(__Header, 1, sizeof(__Header), __Out) == sizeof(__Header)) ? 0 : -1;
}

/* copy_file
 * Role: copy all bytes from input stream to output stream.
 * Params:
 *  - __In: input file stream.
 *  - __Out: output file stream.
 * Return: 0 on success, -1 on read/write error.
 * Author: Jules BELLEVRAS
 */
static int copy_file(FILE *__In, FILE *__Out)
{
    uint8_t __Buf[4096];
    size_t __Read;

    while ((__Read = fread(__Buf, 1, sizeof(__Buf), __In)) > 0)
    {
        if (fwrite(__Buf, 1, __Read, __Out) != __Read)
        {
            return -1;
        }
    }

    return ferror(__In) ? -1 : 0;
}

/* parse_gif_info
 * Role: extract width/height/frame count and a delay from a GIF via ffprobe.
 * Params:
 *  - __GifPath: path to the GIF file.
 *  - __Info: output struct to populate.
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 */
static int parse_gif_info(const char *__GifPath, struct GifInfo *__Info)
{
    char __Cmd[1024];
    char __Line[256];
    FILE *__Pipe = NULL;
    double __FrameDuration = 0.0;
    int __GotDuration = 0;

    __Info->width = 0;
    __Info->height = 0;
    __Info->frames = 0;
    __Info->delay = OLED_DELAY_DEFAULT;

    snprintf(__Cmd, sizeof(__Cmd),
             "ffprobe -v error -select_streams v:0 "
             "-show_entries stream=width,height,nb_frames "
             "-show_entries frame=pkt_duration_time "
             "-of default=noprint_wrappers=1 '%s'",
             __GifPath);

    __Pipe = popen(__Cmd, "r");
    if (__Pipe == NULL)
    {
        perror("ffprobe");
        return -1;
    }

    while (fgets(__Line, sizeof(__Line), __Pipe) != NULL)
    {
        if (sscanf(__Line, "width=%hu", &__Info->width) == 1)
        {
            continue;
        }
        if (sscanf(__Line, "height=%hu", &__Info->height) == 1)
        {
            continue;
        }
        if (sscanf(__Line, "nb_frames=%hu", &__Info->frames) == 1)
        {
            continue;
        }
        if (!__GotDuration && sscanf(__Line, "pkt_duration_time=%lf", &__FrameDuration) == 1)
        {
            __GotDuration = 1;
        }
    }

    pclose(__Pipe);

    if (__Info->frames == 0)
    {
        fprintf(stderr, "ffprobe: nb_frames introuvable.\n");
        return -1;
    }
    if (__Info->width == 0 || __Info->height == 0)
    {
        fprintf(stderr, "ffprobe: width/height introuvable.\n");
        return -1;
    }
    if (__GotDuration && __FrameDuration > 0.0)
    {
        double __DelayCs = __FrameDuration * 100.0;
        if (__DelayCs < 1.0)
        {
            __DelayCs = 1.0;
        }
        if (__DelayCs > 255.0)
        {
            __DelayCs = 255.0;
        }
        __Info->delay = (uint8_t)(__DelayCs + 0.5);
    }

    return 0;
}

/* convert_gif_to_rgb565be
 * Role: convert a GIF to raw RGB565BE using ffmpeg.
 * Params:
 *  - __GifPath: input GIF path.
 *  - __RgbPath: output raw file path.
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 */
static int convert_gif_to_rgb565be(const char *__GifPath, const char *__RgbPath)
{
    char __Cmd[1024];
    int __Ret;

    snprintf(__Cmd, sizeof(__Cmd),
             "ffmpeg -y -i '%s' -vf scale=128:128:flags=neighbor "
             "-pix_fmt rgb565be -f rawvideo '%s' >/dev/null 2>&1",
             __GifPath, __RgbPath);

    __Ret = system(__Cmd);
    if (__Ret != 0)
    {
        fprintf(stderr, "ffmpeg: conversion GIF->RGB565BE echouee.\n");
        return -1;
    }

    return 0;
}

/* build_raw_with_header
 * Role: build a final raw file with OLED header followed by RGB565BE payload.
 * Params:
 *  - __GifPath: input GIF path.
 *  - __RawPath: output raw file path (vf.raw).
 * Return: 0 on success, -1 on error.
 * Author: Jules BELLEVRAS
 * Modified: ChatGPT
 */
int build_raw_with_header(const char *__GifPath, const char *__RawPath)
{
    struct GifInfo __Info;
    FILE *__Header = NULL;
    FILE *__In = NULL;
    FILE *__Out = NULL;
    int __Ret = -1;
    const char *__HeaderPath = "/home/root/GifConvert/header.bin";
    const char *__RgbPath = "/home/root/GifConvert/convert.raw";

    if (parse_gif_info(__GifPath, &__Info) != 0)
    {
        return -1;
    }

    if (convert_gif_to_rgb565be(__GifPath, __RgbPath) != 0)
    {
        return -1;
    }

    __Header = fopen(__HeaderPath, "wb");
    if (__Header == NULL)
    {
        perror("Erreur ouverture header.bin");
        return -1;
    }

    if (write_oled_header(__Header, OLED_WIDTH, OLED_HEIGHT, OLED_BPP, __Info.delay, __Info.frames) != 0)
    {
        fprintf(stderr, "Erreur ecriture header.bin.\n");
        goto cleanup;
    }

    fclose(__Header);
    __Header = NULL;

    __Out = fopen(__RawPath, "wb");
    if (__Out == NULL)
    {
        perror("Erreur ouverture vf.raw");
        goto cleanup;
    }

    __Header = fopen(__HeaderPath, "rb");
    if (__Header == NULL)
    {
        perror("Erreur ouverture header.bin");
        goto cleanup;
    }
    if (copy_file(__Header, __Out) != 0)
    {
        fprintf(stderr, "Erreur copie header.bin -> vf.raw.\n");
        goto cleanup;
    }
    fclose(__Header);
    __Header = NULL;

    __In = fopen(__RgbPath, "rb");
    if (__In == NULL)
    {
        perror("Erreur ouverture convert.raw");
        goto cleanup;
    }
    if (copy_file(__In, __Out) != 0)
    {
        fprintf(stderr, "Erreur copie convert.raw -> vf.raw.\n");
        goto cleanup;
    }

    __Ret = 0;

cleanup:
    if (__Header != NULL)
    {
        fclose(__Header);
    }
    if (__In != NULL)
    {
        fclose(__In);
    }
    if (__Out != NULL)
    {
        fclose(__Out);
    }
    return __Ret;
}
