#include "fctdate.h"

size_t DateLocale(const char* strFormatDate, char* strDate, size_t sztTaillestrDate){
    time_t Time = time(NULL);
    struct tm ActualTime = *localtime(&Time);
    return strftime(strDate,SIZESTRDATE,strFormatDate,&ActualTime);
     
}

