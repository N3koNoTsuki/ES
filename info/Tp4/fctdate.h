/* $Id: fctdate.h, v 1.0 05/07/2017 11:51:52  $
 *
 * Elinux : 
 *
 * Copyright (c) 2009 IUT ANNNECY Departement GEII
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-savoie.fr>
 */

#if !defined __FCTDATE_H__
#define      __FCTDATE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define SIZESTRDATE 60

size_t DateLocale(const char *strFormatDate, char *strDate, size_t sztTaillestrDate);
#endif // #if !defined __FCTDATE_H__
