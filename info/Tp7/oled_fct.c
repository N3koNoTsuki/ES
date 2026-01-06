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

int goldelox_media_init(int fd, uint16_t* out_value)
{
	// Cmd: 0xFF, 0xB1 
	const uint8_t cmd[] = {0xFF, 0xB1};
	if (write_all(fd, cmd, sizeof(cmd)) < 0) return -1;

	// Resp: ACK + value(word) 
	if (goldelox_wait_ack(fd, 300) < 0) return -1;

	uint8_t v[2];
	if (read_exact_timeout(fd, v, 2, 300) < 0) return -1;

	if (out_value) *out_value = (uint16_t)((v[0] << 8) | v[1]);
	return 0;
}

int goldelox_set_sector_address(int fd, uint32_t sector_addr)
{
	// Cmd: 0xFFB8, HIword, LOword 
	uint16_t hi = (uint16_t)(sector_addr >> 16);
	uint16_t lo = (uint16_t)(sector_addr & 0xFFFF);

	uint8_t pkt[2 + 2 + 2];
	pkt[0] = 0xFF; pkt[1] = 0xB8;
	put_u16_be(&pkt[2], hi);
	put_u16_be(&pkt[4], lo);

	if (write_all(fd, pkt, sizeof(pkt)) < 0) return -1;

	// Resp: ACK 
	return goldelox_wait_ack(fd, 300);
}

int goldelox_set_byte_address(int fd, uint32_t byte_addr)
{
	// Cmd: 0xFFB9, HIword, LOword 
	uint16_t hi = (uint16_t)(byte_addr >> 16);
	uint16_t lo = (uint16_t)(byte_addr & 0xFFFF);

	uint8_t pkt[2 + 2 + 2];
	pkt[0] = 0xFF; pkt[1] = 0xB9;
	put_u16_be(&pkt[2], hi);
	put_u16_be(&pkt[4], lo);

	if (write_all(fd, pkt, sizeof(pkt)) < 0) return -1;
	return goldelox_wait_ack(fd, 300);
}

int goldelox_video_frame_raw(int fd, uint16_t x, uint16_t y, uint16_t frame)
{
	// Cmd: 0xFFBA, x, y, frameNumber 
	uint8_t pkt[2 + 2 + 2 + 2];
	pkt[0] = 0xFF; pkt[1] = 0xBA;
	put_u16_be(&pkt[2], x);
	put_u16_be(&pkt[4], y);
	put_u16_be(&pkt[6], frame);

	if (write_all(fd, pkt, sizeof(pkt)) < 0) return -1;

	// Resp: ACK 
	return goldelox_wait_ack(fd, 300);
}


int read_exact_timeout(int fd, uint8_t* buf, size_t len, int timeout_ms)
{
	size_t off = 0;
	while (off < len)
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		struct timeval tv;
		tv.tv_sec  = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;

		int r = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (r < 0)
		{
			if (errno == EINTR) continue;
			return -1;
		}
		if (r == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}

		ssize_t rd = read(fd, buf + off, len - off);
		if (rd < 0)
		{
			if (errno == EINTR) continue;
			return -1;
		}
		if (rd == 0)
		{
			errno = EIO;
			return -1;
		}
		off += (size_t)rd;
	}
	return 0;
}

void put_u16_be(uint8_t out[2], uint16_t v)
{
	out[0] = (uint8_t)(v >> 8);
	out[1] = (uint8_t)(v & 0xFF);
}

int goldelox_wait_ack(int fd, int timeout_ms)
{
	uint8_t ack = 0;
	if (read_exact_timeout(fd, &ack, 1, timeout_ms) < 0) return -1;
	return (ack == 0x06) ? 0 : -1; // 0x06 = ACK 
}