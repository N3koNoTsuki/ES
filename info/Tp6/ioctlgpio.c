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

#include "ioctlgpio.h"

sig_atomic_t sigFlagStop = 0;

void signal_handler(int iSignal);
void signal_handler(int iSignal)
{

	switch (iSignal)
	{
	case SIGINT:
		sigFlagStop = -1;
		break;
	case SIGTERM:
		sigFlagStop = -1;
		break;
	case SIGALRM :
		/* nothing to do */
		break;
	default:
		break;
	}
}

int main(int iArgCount, char *apstrArgValue[])
{

	int iResult = 0;

	struct TDeviceCharGpIO gpioLeds = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip0",
		},
	};
	/*
	// Buttons on Falling Edge (o x x o)
	struct TDeviceCharGpIO gpioButton1 = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip2",
		},
	};
	*/
	/*
	// Buttons on Both Edges (x o o x)
	struct TDeviceCharGpIO gpioButton2 = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip3",
		},
	};
	*/
	
	gpioLeds.m_DevChar.m_iDescIo = open(gpioLeds.m_DevChar.m_strPathDev, O_RDWR);
	if (gpioLeds.m_DevChar.m_iDescIo < 0) {
		perror("open()");
	}
	
	memset(&gpioLeds.m_chipinfo, 0, sizeof(struct gpiochip_info));

	iResult = ioctl(gpioLeds.m_DevChar.m_iDescIo, GPIO_GET_CHIPINFO_IOCTL, &gpioLeds.m_chipinfo);
	if (iResult < 0)
	{
		/* code */
		perror("ioctl()1");
	}
	

	printf("GPIO Chip: %s, %s, %u lines\n",
		gpioLeds.m_chipinfo.name,
		gpioLeds.m_chipinfo.label,
		gpioLeds.m_chipinfo.lines);	

	gpioLeds.m_request.flags = GPIOHANDLE_REQUEST_OUTPUT;
	gpioLeds.m_request.lineoffsets[0] = 9;
	gpioLeds.m_request.lines = 1;

	iResult = ioctl(gpioLeds.m_DevChar.m_iDescIo, GPIO_GET_LINEHANDLE_IOCTL, &gpioLeds.m_request);
	if (iResult < 0)
	{
		/* code */
		perror("ioctl()2");
	}

	
	gpioLeds.m_data.values[0] = 1; // LED ON

	iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
	if (iResult < 0)
	{
		/* code */
		perror("ioctl()3");
	}

	

	return iResult;
}
