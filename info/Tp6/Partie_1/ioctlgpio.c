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
sig_atomic_t sigUsr = 0;

#ifndef USE_REFACTORED_LOGIC
#define USE_REFACTORED_LOGIC 0 /* 1: logique optimisee (Codex), 0: logique originale */
#endif

void signal_handler(int iSignal);
#if USE_REFACTORED_LOGIC
static void clear_line_data(struct gpiohandle_data *data);
static int setup_output_lines(struct TDeviceCharGpIO *gpio, int fd, size_t line_count);
#endif
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
	case SIGUSR1:
		sigUsr = 1;
		break;
	default:
		break;
	}
}

#if USE_REFACTORED_LOGIC
static void clear_line_data(struct gpiohandle_data *data)
{
	memset(data, 0, sizeof(struct gpiohandle_data));
}

static int setup_output_lines(struct TDeviceCharGpIO *gpio, int fd, size_t line_count)
{
	memset(&gpio->m_request, 0, sizeof(gpio->m_request));
	gpio->m_request.flags = GPIOHANDLE_REQUEST_OUTPUT;
	gpio->m_request.lines = line_count;
	for (size_t i = 0; i < line_count; i++)
	{
		gpio->m_request.lineoffsets[i] = i;
	}

	return ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &gpio->m_request);
}
#endif

int main(int iArgCount, char *apstrArgValue[])
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGALRM, signal_handler);
	signal(SIGUSR1, signal_handler);

	int iResult = 0;
	pid_t pid;

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

	pid = fork();
	switch (pid)
	{
	case -1:
		perror("Echec fork()");
		break;
	case 0:
		// code du fils
		ualarm(50000, 50000);
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

#if USE_REFACTORED_LOGIC
		{
			int chenillardActive = 0;
			int direction = 1;
			int position = 0;

			iResult = setup_output_lines(&gpioLeds, gpioLeds.m_DevChar.m_iDescIo, 10);
			if (iResult < 0)
			{
				perror("ioctl() ledsRequest");
			}
			clear_line_data(&gpioLeds.m_data);
			// animation chenillard
			while (sigFlagStop != -1)
			{
				if (sigUsr)
				{
					sigUsr = 0;
					chenillardActive = !chenillardActive;
				}

				if (!chenillardActive)
				{
					pause();
					continue;
				}

				gpioLeds.m_data.values[position] = 1;
				if (direction == 1 && position > 0)
				{
					gpioLeds.m_data.values[position - 1] = 0;
				}
				else if (direction == -1 && position < 9)
				{
					gpioLeds.m_data.values[position + 1] = 0;
				}

				iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
				if (iResult < 0)
				{
					perror("ioctl() chenillard");
				}

				pause();

				if (sigFlagStop == -1)
				{
					break;
				}

				if ((direction == 1 && position == 9) || (direction == -1 && position == 0))
				{
					direction = -direction;
				}
				position += direction;
			}
			ualarm(0, 0);
			clear_line_data(&gpioLeds.m_data);
		}
#else
		{
			int cycle = 0;

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
			// animation chenillard
			while (sigFlagStop != -1)
			{
				if (sigUsr)
				{
					sigUsr = 0;
					cycle = (cycle + 1) % 2;
				}

				if (cycle == 0)
				{
					pause();
					continue;
				}

				for (size_t i = 0; i < 10 && sigFlagStop != -1; ++i)
				{
					if (sigUsr)
					{
						sigUsr = 0;
						cycle = (cycle + 1) % 2;
					}

					if (sigFlagStop == -1)
					{
						break;
					}
					if (cycle == 0)
					{
						pause();
						i = i - 1;
						continue;
					}
					else
					{
						gpioLeds.m_data.values[i] = 1;
						if (i > 0)
						{
							gpioLeds.m_data.values[i - 1] = 0;
						}

						iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
						if (iResult < 0)
						{
							perror("ioctl() chenillard forward");
						}
						pause();
					}
				}

				if (sigFlagStop == -1)
				{
					break;
				}
				if (cycle == 0)
				{
					pause();
					continue;
				}

				for (int i = 9; i >= 0 && sigFlagStop != -1; --i)
				{
					if (sigUsr)
					{
						sigUsr = 0;
						cycle = (cycle + 1) % 2;
					}

					if (sigFlagStop == -1)
					{
						break;
					}
					if (cycle == 0)
					{
						pause();
						i = i + 1;
						continue;
					}
					else
					{
						gpioLeds.m_data.values[i] = 1;
						if (i < 9)
						{
							gpioLeds.m_data.values[i + 1] = 0;
						}

						iResult = ioctl(gpioLeds.m_request.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &gpioLeds.m_data);
						if (iResult < 0)
						{
							perror("ioctl() chenillard backward");
						}
					}
					pause();
				}
			}
			kill(ualarm(0, 0), SIGALRM);
			for (size_t i = 0; i < 10; i++)
			{
				gpioLeds.m_data.values[i] = 0;
			}
		}
#endif

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
#if USE_REFACTORED_LOGIC
		gpioButton1.m_DevChar.m_iDescIo = open(gpioButton1.m_DevChar.m_strPathDev, O_RDWR);
		if (gpioButton1.m_DevChar.m_iDescIo < 0)
		{
			perror("open() button1");
		}
		memset(&gpioButton1.m_request, 0, sizeof(gpioButton1.m_request));
		memset(&gpioButton1.m_data, 0, sizeof(gpioButton1.m_data));
		memset(gpioButton1.m_event_request, 0, sizeof(gpioButton1.m_event_request));
		memset(gpioButton1.m_event_data, 0, sizeof(gpioButton1.m_event_data));
		memset(gpioButton1.m_pollfd, 0, sizeof(gpioButton1.m_pollfd));

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
				if (sigFlagStop == -1)
				{
					break;
				}
				else
				{
					perror("poll() button1 event");
				}
			}
			else
			{
				read(gpioButton1.m_pollfd[0].fd, &gpioButton1.m_event_data[0], sizeof(struct gpioevent_data));
				if (gpioButton1.m_event_data[0].id == GPIOEVENT_EVENT_FALLING_EDGE)
				{
					kill(pid, SIGUSR1);
				}
			}
		}

		printf("\r%s: Fin\n", apstrArgValue[0]);
		close(gpioButton1.m_event_request[0].fd);
		close(gpioButton1.m_DevChar.m_iDescIo);
		int status = 0;
		pid_t w = waitpid(pid, &status, 0);
		if (w == -1)
		{
			perror("waitpid");
		}
		else
		{
		}
#else
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
				if (sigFlagStop == -1)
				{
					break;
				}
				else
				{
					perror("poll() button1 event");
				}
			}
			else
			{
				read(gpioButton1.m_pollfd[0].fd, &gpioButton1.m_event_data, sizeof(struct gpioevent_data));
				if (gpioButton1.m_event_data->id == GPIOEVENT_EVENT_FALLING_EDGE)
				{
					kill(pid, SIGUSR1);
				}
			}
		}

		printf("\r%s: Fin\n", apstrArgValue[0]);
		close(gpioButton1.m_request.fd);
		close(gpioButton1.m_DevChar.m_iDescIo);
		int status = 0;
		pid_t w = waitpid(pid, &status, 0);
		if (w == -1)
		{
			perror("waitpid");
		}
		else
		{
		}
#endif
		break;
	}

	return iResult;
}
