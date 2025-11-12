/* ---------------------------------------------------------------------
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ioctlgpio.c
 *
 *   This is a small example of ioctl() accesses to the  GPIO user-space
 *   API under Linux (list the GPIO controllers and theirs lines,
 *   to read/write data, to wait on a input GPIO line for some events.
 *
 * Copyright (C) 2023 GEii IUT-Acy, USMB.
 *
 * Author:
 * 	Pascal Aygalinc <pascal.aygalinc@univ-smb.fr>
 * ---------------------------------------------------------------------
 */

#include <stdio.h>   
#include <string.h>  
#include <unistd.h>  
#include <fcntl.h>   
#include <errno.h>   
#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <poll.h>

#include "ioctlgpio.h"

sig_atomic_t  sigFlagStop=0;

void signal_handler(int iSignal);
void signal_handler(int iSignal){

	switch (iSignal) {

		default:
				break;
	}
}




int main(int iArgCount, char* apstrArgValue[]){

	int iResult=0;
	
	struct TDeviceCharGpIO gpioLeds 		=	{	.m_DevChar	= {	.m_iDescIo 		= -1,
																					.m_strPathDev 	= "/dev/gpiochip0", },
														};
	/*
	// Buttons on Falling Edge (o x x o)												
	struct TDeviceCharGpIO gpioButton1 	= 	{	.m_DevChar	= {	.m_iDescIo 		= -1,
																					.m_strPathDev 	= "/dev/gpiochip2", },
														};
	*/
	/*
	// Buttons on Both Edges (x o o x)							
	struct TDeviceCharGpIO gpioButton2 	= 	{	.m_DevChar	= {	.m_iDescIo 		= -1,
																					.m_strPathDev 	= "/dev/gpiochip3", },
														};	
	*/			

	
	
	return iResult;
}
	
	
	


