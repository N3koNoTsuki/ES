/* OLED GIF uploader/reader definitions */
#if !defined __TEST_H__
#define      __TEST_H__ 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <wait.h>
#include <linux/gpio.h>

#define SIZE 60000

#define SIZESTRADC 60
#define GIF_ADDRESS_PATH "/home/root/GifConvert/gif_addresses.txt"
#define BYTES_PER_SECTOR 512
#define BYTES_PER_LINE 16
#define BYTES_PER_SECTOR 512
#define BYTES_PER_LINE 16
#define OLED_WIDTH 128
#define OLED_HEIGHT 128
#define OLED_BPP 16
#define OLED_DELAY_DEFAULT 0x1E


struct GifInfo {
    uint16_t width;
    uint16_t height;
    uint16_t frames;
    uint8_t delay;
};

struct TCharDevice {
	char 	m_strFileIO[60];
	int		m_iDescIO;
};

struct TDeviceCharGpIO;
struct TDeviceCharGpIO {
	struct TCharDevice 			m_DevChar;
	struct gpiochip_info 		m_chipinfo;
	struct gpioline_info 		m_lineinfo;
	struct gpiohandle_request 	m_request;
	struct gpiohandle_data 		m_data;
	struct gpioevent_request  	m_event_request[64];
	struct gpioevent_data     	m_event_data[64];
	struct pollfd 					m_pollfd[64];
};

enum BaudRate {
    BAUDRATE_9600 = 9600,
    BAUDRATE_115200 = 115200
};
enum DebugLevel {
    DEBUG_NONE,
    DEBUG_ERROR,
    DEBUG_INFO,
    DEBUG_VERBOSE
};


int ClearOled(int __Fd);
int SendCmd(int __Fd, const uint8_t * __Buf, size_t __Len);
int SendCmd2RetV(int __Fd, const uint8_t * __Buf, size_t __Len, uint16_t *__Ret);
int change_baudrate(struct TCharDevice *__Dev, struct termios* __Term, enum BaudRate __BaudRate, enum DebugLevel __DbgLvl);
int init_liaison(struct TCharDevice *__Dev, struct termios *__Init, struct termios *__Old, size_t __lenInit);
int init_media(int __Fd, uint16_t *__Ret);
int GetNbFrames(struct TCharDevice *__Fds, uint8_t __CmdSetByte[], uint16_t* __NbFrames);
int Write_Byte_media(struct TCharDevice *__Fds, uint8_t __Lsb);
int Write_Word_media(struct TCharDevice *__Fds, uint8_t __Msb, uint8_t __Lsb);
int build_raw_with_header(const char *__GifPath, const char *__RawPath);
void print_progress(size_t __Written, size_t __Total);
int LoadGifAddresses(const char *__Path, uint8_t (**__CmdSetByte)[6], size_t *__OutCount);
int LoadGifFreeAddress(const char *__Path, uint8_t __CmdSetByte[6]);
int AppendGifAddressFromRaw(const char *__AddrPath, const char *__RawPath, uint32_t *__OutAddress);
#endif // #if !defined __TEST_H__
