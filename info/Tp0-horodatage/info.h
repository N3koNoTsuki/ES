/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * linux : Affiche les information de la date de l'utilisateur
 *
 * Copyright (c) 2017 IUT ANNNECY Departement GEII/ESE
 */

#ifndef      __INFO_H__
#define      __INFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define SIZESTRDATE 50



typedef struct
{
    time_t TimeNow;
    struct tm ActualTime;
    char MonHeure[SIZESTRDATE];
}TTime;


void GetTimeNow(TTime* Time){
    Time->TimeNow = time(NULL);
	Time->ActualTime = *localtime(&Time->TimeNow);   
}

void Getheure(TTime* Time){
    GetTimeNow(Time);   
    strftime(Time->MonHeure,50,"%A %d,%B %Y %T %Z\n",&Time->ActualTime);
    printf(Time->MonHeure);
}

void clearLastLine() {
    printf("\033[A");  
    printf("\033[2K"); 
    fflush(stdout);    
}

#endif 

