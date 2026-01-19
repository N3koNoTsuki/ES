/* $Id: oled.h, v 1.0 05/07/2017 11:51:52  $
 *
 * Elinux : 
 *
 * Copyright (c) 2017 IUT ANNNECY Departement GEII
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-savoie.fr>
 */

#if !defined __OLED_H__
#define      __OLED_H__ 

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
#define NBGIF 3


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
int SendCmd2RetV(int __Fd, const uint8_t * __Buf, uint16_t *__Ret);
int change_baudrate(struct TCharDevice *__Dev, struct termios* __Term, enum BaudRate __BaudRate, enum DebugLevel __DbgLvl);
int init_liaison(struct TCharDevice *__Dev, struct termios *__Init, struct termios *__Old, size_t __lenInit);
int init_media(int __Fd, uint16_t *__Ret);
int config_gpio_button(struct TDeviceCharGpIO* __Fds, unsigned int __EventFlgs, enum DebugLevel __DbgLvl);
int button_press(struct TDeviceCharGpIO *__Fds, int __Idx);
int check_all_buttons(struct TDeviceCharGpIO* __Fds1, struct TDeviceCharGpIO* __Fds2);
int GstPipe(int *__Gif, char __Idx );
int GetNbFrames(struct TCharDevice *__Fds, uint8_t __CmdSetByte[], uint16_t* __NbFrames);
#endif // #if !defined __OLED_H__
