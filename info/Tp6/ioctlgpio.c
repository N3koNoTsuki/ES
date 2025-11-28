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
	case SIGALRM:
		/* nothing to do */
		break;
	default:
		break;
	}
}

int main(int iArgCount, char *apstrArgValue[])
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGALRM, signal_handler);

	int iResult = 0;
	pid_t pid;
	int a2iFdPipe[2];
	int cycle = 0;

	struct TDeviceCharGpIO gpioLeds = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip0",
		},
	};

	// Buttons on Falling Edge (o x x o)
	struct TDeviceCharGpIO gpioButton1 = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip2",
		},
	};

	/*
	// Buttons on Both Edges (x o o x)
	struct TDeviceCharGpIO gpioButton2 = {
		.m_DevChar = {
			.m_iDescIo = -1,
			.m_strPathDev = "/dev/gpiochip3",
		},
	};
	*/
	iResult = pipe(a2iFdPipe);
	if (iResult == 0)
	{
		pid = fork();
		switch (pid)
		{
		case -1:
			perror("Echec fork()");
			close(a2iFdPipe[0]);
			close(a2iFdPipe[1]);
			break;
		case 0:
			// code du fils
			close(a2iFdPipe[1]);
			ualarm(50000, 50000);
			// animation chenillard
			while (sigFlagStop != -1)
			{
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
				}
				gpioLeds.m_request.lines = 10;

				iResult = ioctl(gpioLeds.m_DevChar.m_iDescIo, GPIO_GET_LINEHANDLE_IOCTL, &gpioLeds.m_request);
				if (iResult < 0)
				{
					/* code */
					perror("ioctl() ledsRequest");
				}

				for (size_t i = 0; i < 10; i++)
				{
					if (sigFlagStop == -1 || cycle == 0)
					{
						break;
					}
					gpioLeds.m_data.values[i] = 1;

					if (i > 0)
					{
						gpioLeds.m_data.values[i - 1] = 0;
					}
					iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
					if (iResult < 0)
					{
						/* code */
						perror("ioctl() chenillard");
					}
					pause();
				}
				// sens inverse
				for (int i = 9; i >= -1; i--)
				{
					if (sigFlagStop == -1 || cycle == 0)
					{
						break;
					}
					if (i >= 0)
					{
						gpioLeds.m_data.values[i] = 1;
					}
					if (i < 9)
					{
						gpioLeds.m_data.values[i + 1] = 0;
					}
					iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
					if (iResult < 0)
					{
						perror("ioctl() chennillardinverse");
					}
					pause();
				}
			}

			kill(ualarm(0, 0), SIGALRM);
			for (size_t i = 0; i < 10; i++)
			{
				gpioLeds.m_data.values[i] = 0;
			}

			iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
			if (iResult < 0)
			{
				/* code */
				perror("ioctl() ledson");
			}
			close(gpioLeds.m_request.fd);
			close(gpioLeds.m_DevChar.m_iDescIo);
			kill(getppid(), SIGINT);

			break;
		default:
			// code du pere
			close(a2iFdPipe[0]);
			gpioButton1.m_DevChar.m_iDescIo = open(gpioButton1.m_DevChar.m_strPathDev, O_RDWR);
			if (gpioButton1.m_DevChar.m_iDescIo < 0)
			{
				perror("open() button1");
			}

			memset(&gpioButton1.m_chipinfo, 0, sizeof(struct gpiochip_info));

			iResult = ioctl(gpioButton1.m_DevChar.m_iDescIo, GPIO_GET_CHIPINFO_IOCTL, &gpioButton1.m_chipinfo);
			if (iResult < 0)
			{
				perror("ioctl() button1");
			}

			printf("GPIO Chip: %s, %s, %u lines\n",
				   gpioButton1.m_chipinfo.name,
				   gpioButton1.m_chipinfo.label,
				   gpioButton1.m_chipinfo.lines);

			gpioButton1.m_event_request[0].lineoffset = 0;
			gpioButton1.m_event_request[0].handleflags = GPIOHANDLE_REQUEST_INPUT;
			gpioButton1.m_event_request[0].eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;

			iResult = ioctl(gpioButton1.m_DevChar.m_iDescIo, GPIO_GET_LINEEVENT_IOCTL, &gpioButton1.m_event_request[0]);
			if (iResult < 0)
			{
				perror("ioctl() button1Request");
			}

			gpioButton1.m_pollfd[0].fd = gpioButton1.m_event_request[0].fd;
			gpioButton1.m_pollfd[0].events = POLLIN;
			while (sigFlagStop != -1)
			{
				/* Wait for button press event */
				iResult = poll(&gpioButton1.m_pollfd[0], 1, -1);
				if (iResult < 0)
				{
					perror("poll() button1 event");
					cycle = 0;
				}
				else
				{
					read(gpioButton1.m_pollfd[0].fd, &gpioButton1.m_event_data, sizeof(struct gpioevent_data));
					if (gpioButton1.m_event_data->id == GPIOEVENT_EVENT_FALLING_EDGE)
					{
						/* code */
						printf("Button1 pressed !\n");
						cycle = (cycle + 1) % 2;
					}
				}
			}

			printf("\n%s>Fin\n", apstrArgValue[0]);
			close(gpioButton1.m_request.fd);
			close(gpioButton1.m_DevChar.m_iDescIo);
			int status = 0;
			close(a2iFdPipe[0]);
			close(a2iFdPipe[1]);
			pid_t w = waitpid(pid, &status, 0);
			if (w == -1)
			{
				perror("waitpid");
			}
			else
			{
			}
			break;
		}
	}
	return iResult;
}
