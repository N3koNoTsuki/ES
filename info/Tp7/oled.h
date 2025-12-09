/* $Id: oled.h, v 1.0 05/07/2017 11:51:52  $
 *
 * Elinux : 
 *
 * Copyright (c) 2017 IUT ANNNECY Departement GEII
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-savoie.fr>
 */

#if !defined __OLED_H__
#define      __OLED_H__
//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

struct TCharDevice {
	char 	m_strFileIO[60];
	int		m_iDescIO;
};

void clearcmd(uint8_t* cmd, size_t len);
int CleaOled(int __Fd);
int SendCmd(int __Fd, const uint8_t * __buf, size_t len);
#endif // #if !defined __OLED_H__
