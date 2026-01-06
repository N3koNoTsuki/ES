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
#include <math.h>

#define OLED_CMD_CIRCLE_LEN 10
#define OLED_CMD_LINE_LEN 12
#define OLED_CMD_CHANGE_COLOR_LEN 6

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct TCharDevice {
	char 	m_strFileIO[60];
	int		m_iDescIO;
};



void clearcmd(uint8_t* cmd, size_t len);
int CleaOled(int __Fd);
int SendCmd(int __Fd, const uint8_t * __buf, size_t len);
void build_circle_cmd(uint8_t *cmd, uint8_t opcode, uint16_t x, uint16_t y, uint16_t r, uint16_t color);
void build_line_cmd(uint8_t *cmd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Build_ChangeColor_cmd(uint8_t *cmd, uint16_t __Old, uint16_t __New);
int goldelox_media_init(int fd, uint16_t* out_value);
int goldelox_set_sector_address(int fd, uint32_t sector_addr);
int goldelox_set_byte_address(int fd, uint32_t byte_addr);
int goldelox_video_frame_raw(int fd, uint16_t x, uint16_t y, uint16_t frame);
int read_exact_timeout(int fd, uint8_t* buf, size_t len, int timeout_ms);
void put_u16_be(uint8_t out[2], uint16_t v);
int goldelox_wait_ack(int fd, int timeout_ms);

#endif // #if !defined __OLED_H__
