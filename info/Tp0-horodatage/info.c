/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * linux : Affiche l'heure HH:MN sur le flux stdout
 *
 * Copyright (c) 2023 IUT ANNNECY Departement GEII/ESE
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 */
 

#include "info.h"

int main(int iArgCount, char* apstrArgValue[]){

	// reservation
	int iResult=0;
	
	TTime Mytime;
	TTime MemMytime;
	
	// instructions
	Getheure(&Mytime);
	MemMytime = Mytime;
	
	while (1==1){
		
		if (MemMytime.ActualTime.tm_sec != Mytime.ActualTime.tm_sec)
		{	
			clearLastLine();
			Getheure(&Mytime);
			MemMytime = Mytime;
		}
		else{
			GetTimeNow(&Mytime);
		}
		
	
	}
	

	
	return iResult;
}
