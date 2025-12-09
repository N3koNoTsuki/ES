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
#include <stdbool.h>
#include <stdint.h>

sig_atomic_t sigFlagStop = 0;



void signal_handler(int iSignal);

static void set_all_leds(struct gpiohandle_data *data, uint8_t value);
static void configure_button_event(struct TDeviceCharGpIO *gpio, size_t idx, unsigned int lineoffset);
static void refresh_button_state(struct TDeviceCharGpIO *gpio, size_t idx, bool *pressed);

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
	case SIGALRM:
		/* nothing to do */
		break;
	default:
		break;
	}
}

static void set_all_leds(struct gpiohandle_data *data, uint8_t value)
{
	for (size_t i = 0; i < 10; i++)
	{
		data->values[i] = value;
	}
}

static void configure_button_event(struct TDeviceCharGpIO *gpio, size_t idx, unsigned int lineoffset)
{
	gpio->m_event_request[idx].lineoffset = lineoffset;
	gpio->m_event_request[idx].handleflags = GPIOHANDLE_REQUEST_INPUT;
	gpio->m_event_request[idx].eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
}

static void refresh_button_state(struct TDeviceCharGpIO *gpio, size_t idx, bool *pressed)
{
	if (!(gpio->m_pollfd[idx].revents & POLLIN))
	{
		return;
	}

	if (read(gpio->m_pollfd[idx].fd, &gpio->m_event_data[idx], sizeof(struct gpioevent_data)) < 0)
	{
		perror("read() button event");
		return;
	}

	if (gpio->m_event_data[idx].id == GPIOEVENT_EVENT_FALLING_EDGE)
	{
		*pressed = true;
	}
	else if (gpio->m_event_data[idx].id == GPIOEVENT_EVENT_RISING_EDGE)
	{
		*pressed = false;
	}
}


int main(int iArgCount, char *apstrArgValue[])
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGALRM, signal_handler);
	int iResult = 0;

	bool bButton0Pressed = false;
	bool bButton1Pressed = false;
	bool bBlinkStateOn = false;

	struct TDeviceCharGpIO gpioLeds = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip0",
		},
	};
	struct TDeviceCharGpIO gpioButton2 = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip3",
		},
	};
	memset(&gpioLeds.m_request, 0, sizeof(gpioLeds.m_request));
	memset(&gpioLeds.m_data, 0, sizeof(gpioLeds.m_data));
	memset(gpioButton2.m_event_request, 0, sizeof(gpioButton2.m_event_request));
	memset(gpioButton2.m_event_data, 0, sizeof(gpioButton2.m_event_data));
	memset(gpioButton2.m_pollfd, 0, sizeof(gpioButton2.m_pollfd));

	/*----------------------------LEDS SETUP--------------------------------*/
	gpioLeds.m_DevChar.m_iDescIo = open(gpioLeds.m_DevChar.m_strPathDev, O_RDWR);
	if (gpioLeds.m_DevChar.m_iDescIo < 0)
	{
		perror("open() leds");
	}
	memset(&gpioLeds.m_chipinfo, 0, sizeof(struct gpiochip_info));
	iResult = ioctl(gpioLeds.m_DevChar.m_iDescIo, GPIO_GET_CHIPINFO_IOCTL, &gpioLeds.m_chipinfo);
	if (iResult < 0)
	{
		perror("ioctl() leds");
	}
	printf("GPIO Chip: %s, %s, %u lines\n",
		   gpioLeds.m_chipinfo.name,
		   gpioLeds.m_chipinfo.label,
		   gpioLeds.m_chipinfo.lines);
	gpioLeds.m_request.flags = GPIOHANDLE_REQUEST_OUTPUT;
	for (size_t i = 0; i < 10; i++)
	{
		gpioLeds.m_request.lineoffsets[i] = i;
		gpioLeds.m_request.default_values[i] = 0;
	}
	gpioLeds.m_request.lines = 10;
	iResult = ioctl(gpioLeds.m_DevChar.m_iDescIo, GPIO_GET_LINEHANDLE_IOCTL, &gpioLeds.m_request);
	if (iResult < 0)
	{
		perror("ioctl() ledsRequest");
	}

	/*----------------------------BUTTON  SETUP--------------------------------*/
	gpioButton2.m_DevChar.m_iDescIo = open(gpioButton2.m_DevChar.m_strPathDev, O_RDWR);
	if (gpioButton2.m_DevChar.m_iDescIo < 0)
	{
		perror("open() button2");
	}
	memset(&gpioButton2.m_chipinfo, 0, sizeof(struct gpiochip_info));
	iResult = ioctl(gpioButton2.m_DevChar.m_iDescIo, GPIO_GET_CHIPINFO_IOCTL, &gpioButton2.m_chipinfo);
	if (iResult < 0)
	{
		perror("ioctl() button2");
	}
	printf("GPIO Chip: %s, %s, %u lines\n",
		   gpioButton2.m_chipinfo.name,
		   gpioButton2.m_chipinfo.label,
		   gpioButton2.m_chipinfo.lines);

	/*----------------------------BUTTON 1 SETUP--------------------------------*/

	configure_button_event(&gpioButton2, 0, 0);

	iResult = ioctl(gpioButton2.m_DevChar.m_iDescIo, GPIO_GET_LINEEVENT_IOCTL, &gpioButton2.m_event_request[0]);
	if (iResult < 0)
	{
		perror("ioctl() button2[0]Request");
	}

	/*----------------------------BUTTON 2 SETUP--------------------------------*/

	configure_button_event(&gpioButton2, 1, 1);

	iResult = ioctl(gpioButton2.m_DevChar.m_iDescIo, GPIO_GET_LINEEVENT_IOCTL, &gpioButton2.m_event_request[1]);
	if (iResult < 0)
	{
		perror("ioctl() button2[1]Request");
	}

	/*----------------------------POLL SETUP--------------------------------*/
	gpioButton2.m_pollfd[0].fd = gpioButton2.m_event_request[0].fd;
	gpioButton2.m_pollfd[0].events = POLLIN;
	gpioButton2.m_pollfd[1].fd = gpioButton2.m_event_request[1].fd;
	gpioButton2.m_pollfd[1].events = POLLIN;

	/*----------------------------MAIN LOOP--------------------------------*/
	while (sigFlagStop != -1)
	{
		int iTimeoutMs = (bButton1Pressed && !bButton0Pressed) ? 500 : -1;
		int iPollRes = poll(gpioButton2.m_pollfd, 2, iTimeoutMs);
		if (iPollRes < 0)
		{
			if (sigFlagStop == -1)
			{
				break;
			}
			else
			{
				perror("poll() button event");
				continue;
			}
		}

		refresh_button_state(&gpioButton2, 0, &bButton0Pressed);
		refresh_button_state(&gpioButton2, 1, &bButton1Pressed);

		if (iPollRes == 0 && bButton1Pressed && !bButton0Pressed)
		{
			bBlinkStateOn = !bBlinkStateOn;
		}

		if (bButton0Pressed)
		{
			set_all_leds(&gpioLeds.m_data, 1);
		}
		else if (bButton1Pressed)
		{
			set_all_leds(&gpioLeds.m_data, bBlinkStateOn ? 1 : 0);
		}
		else
		{
			set_all_leds(&gpioLeds.m_data, 0);
		}

		if (ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data) < 0)
		{
			perror("ioctl() ledson");
		}
	}


	set_all_leds(&gpioLeds.m_data, 0);

	iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
	if (iResult < 0)
	{
		perror("ioctl() ledson");
	}
	close(gpioLeds.m_request.fd);
	close(gpioLeds.m_DevChar.m_iDescIo);
	close(gpioButton2.m_event_request[0].fd);
	close(gpioButton2.m_event_request[1].fd);
	close(gpioButton2.m_DevChar.m_iDescIo);
	printf("\r%s: Fin\n", apstrArgValue[0]);
	return iResult;
}
