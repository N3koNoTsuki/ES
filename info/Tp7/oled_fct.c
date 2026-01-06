#include "oled.h"

static int write_all(int fd, const uint8_t *buf, size_t len)
{
	size_t written_total = 0;
	while (written_total < len)
	{
		ssize_t written = write(fd, buf + written_total, len - written_total);
		if (written < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			return -1;
		}
		written_total += (size_t)written;
	}
	return 0;
}

static int read_ack_byte(int fd, uint8_t *ack)
{
	ssize_t read_len;
	do
	{
		read_len = read(fd, ack, sizeof(*ack));
	} while (read_len < 0 && errno == EINTR);

	if (read_len < 0)
	{
		return -1;
	}
	if (read_len == 0)
	{
		errno = EIO;
		return -1;
	}
	return 0;
}

void clearcmd(uint8_t* cmd, size_t len){
    memset(cmd, 0, len);
}

int CleaOled(int __Fd){
    const uint8_t cmd[2] = {0xFF, 0xD7};
    uint8_t RetVal = 0;

	if (write_all(__Fd, cmd, sizeof(cmd)) < 0)
	{
		return -1;
	}
	if (read_ack_byte(__Fd, &RetVal) < 0)
	{
		return -2;
	}

    return RetVal;
}

int SendCmd(int __Fd, const uint8_t * __buf, size_t len){

    uint8_t RetVal = 0;

    #ifdef DEBUG
    #endif

	if (write_all(__Fd, __buf, len) < 0)
	{
		#ifdef DEBUG
		perror("write ");
		#endif
		return -1; // propagate write error
	}

	if (read_ack_byte(__Fd, &RetVal) < 0)
	{
		#ifdef DEBUG
		perror("read ");
		#endif
		return -2; // propagate read error
	}

	return RetVal;
}


void build_circle_cmd(uint8_t *cmd, uint8_t opcode, uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
	cmd[0] = 0xFF;
	cmd[1] = opcode;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (x >> 8) & 0xFF;
	cmd[3] = x & 0xFF;
	cmd[4] = (y >> 8) & 0xFF;
	cmd[5] = y & 0xFF;
	cmd[6] = (r >> 8) & 0xFF;
	cmd[7] = r & 0xFF;
	cmd[8] = (color >> 8) & 0xFF;
	cmd[9] = color & 0xFF;
}

void build_line_cmd(uint8_t *cmd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    cmd[0] = 0xFF;
	cmd[1] = 0xD2;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (x1 >> 8) & 0xFF;
	cmd[3] = x1 & 0xFF;
	cmd[4] = (y1 >> 8) & 0xFF;
	cmd[5] = y1 & 0xFF;
    cmd[6] = (x2 >> 8) & 0xFF;
	cmd[7] = x2 & 0xFF;
	cmd[8] = (y2 >> 8) & 0xFF;
	cmd[9] = y2 & 0xFF;
	cmd[10] = (color >> 8) & 0xFF;
	cmd[11] = color & 0xFF;
}


void Build_ChangeColor_cmd(uint8_t *cmd, uint16_t __Old, uint16_t __New)
{
    cmd[0] = 0xFF;
	cmd[1] = 0xBE;		 // 0xCD: empty circle, 0xCC: filled
	cmd[2] = (__Old >> 8) & 0xFF;
	cmd[3] = __Old & 0xFF;
	cmd[4] = (__New >> 8) & 0xFF;
	cmd[5] = __New & 0xFF;
}