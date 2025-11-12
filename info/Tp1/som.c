#include "fctdate.h"




int main(int iArgCount, char* apstrArgValue[]){

	// reservation
	int iResult=0;
	
   char OutFormat[10] = {"%T\n"};
   char OutDate[SIZESTRDATE];
   char BufferOutDate[SIZESTRDATE];

   DateLocale(OutFormat, OutDate, SIZESTRDATE);
   printf(OutDate);
   strcpy(BufferOutDate, OutDate);

   while (1)
   {
	   
	if (strcmp(OutDate, BufferOutDate) > 0)
	{
		DateLocale(OutFormat, OutDate, SIZESTRDATE);
		printf("\033[A");  
    	printf("\033[2K"); 
    	fflush(stdout); 
   		printf(OutDate);
   		strcpy(BufferOutDate, OutDate);
		sleep(1);
	}
	else{
		DateLocale(OutFormat, OutDate, SIZESTRDATE);
	}
   }
   



	

	
	return iResult;
}

