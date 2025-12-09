#include "oled.h"

void clearcmd(uint8_t* cmd, size_t len){
    memset(cmd, 0, len);
}

<<<<<<< ours
// Copie une séquence de bytes passés en paramètres variadiques.
size_t fillcmd_bytes(uint8_t* dest, size_t dest_len, size_t count, ...){
    if (dest == NULL || dest_len == 0 || count == 0){
        return 0;
    }

    va_list ap;
    va_start(ap, count);
    size_t copied = 0;
    for (; copied < count && copied < dest_len; ++copied){
        int v = va_arg(ap, int); // les arguments variadiques sont promus en int
        dest[copied] = (uint8_t)v;
    }
    va_end(ap);
    return copied;
}

// Copie une séquence déjà construite.
size_t fillcmd(uint8_t* dest, size_t dest_len, const uint8_t* src, size_t src_len){
    if (dest == NULL || src == NULL || dest_len == 0){
        return 0;
    }

    size_t to_copy = (src_len < dest_len) ? src_len : dest_len;
    memcpy(dest, src, to_copy);
    return to_copy;
}

=======
>>>>>>> theirs
int CleaOled(int __Fd){
    uint8_t cmd[2] = {0xFF, 0xD7};
    int RetVal;

	if (write(__Fd, cmd, sizeof(cmd)) < 0)
	{
		return -1;
	}

	if (read(__Fd, &RetVal, sizeof(RetVal)) < 0)
	{
		return -2;
	}

    return RetVal;
}

int SendCmd(int __Fd, const uint8_t * __buf, size_t len){

    int RetVal;

    #ifdef DEBUG
    fprintf(stdout, "%i\n\r", __buf[1]);
    fprintf(stdout, "HI\n\r");
    #endif

	ssize_t written = write(__Fd, __buf, len);
	if (written < 0)
	{
		return -1;
	}

    #ifdef DEBUG
    perror("write :");
    fprintf(stdout, "HI2\n\r");
    #endif

	ssize_t read_len = read(__Fd, &RetVal, sizeof(RetVal));
	if (read_len < 0)
	{
		return -2;
	}

    #ifdef DEBUG
    perror("read :");
    #endif
    
    return RetVal;
}
