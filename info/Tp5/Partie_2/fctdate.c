/* $Id: fctdate.c, v 1.0 05/07/2017 11:51:52  $
 *
 * linux : heure locale  HH:MN 
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 */

#include "fctdate.h"

size_t DateLocale(const char strFormatDate[], char strDate[], size_t sztTailleStrDate){

	/* reservation et initialisation */
	size_t 		size_tResult=0;
	time_t 		timeNbrSec_01_01_1970=(time_t) 0;
	struct tm* 	ptmDateLocale=(struct tm*) NULL;

	time(&timeNbrSec_01_01_1970);
	ptmDateLocale=localtime(&timeNbrSec_01_01_1970);
	size_tResult=strftime(strDate, sztTailleStrDate, strFormatDate, ptmDateLocale);

	return (size_tResult);
}
