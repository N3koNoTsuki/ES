#if !defined __INFO_H
#define __INFO_H

#define SIZESTRDATE 80
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define SIZESTRDATE 80

size_t DateLocale(const char* strFormatDate, char* strDate, size_t sztTaillestrDate);

#endif
