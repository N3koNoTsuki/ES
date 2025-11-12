/* $Id: fctdate.c, v 1.0 05/07/2017 11:51:52  $
 *
 * linux : heure locale  HH:MN 
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 */



#include "fctdate.h"


size_t DateLocale(const char *strFormatDate, char *strDate, size_t sztTaillestrDate)
{
	time_t Time = time(NULL);
	struct tm ActualTime = *localtime(&Time);
	return strftime(strDate, SIZESTRDATE, strFormatDate, &ActualTime);
}
