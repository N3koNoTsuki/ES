/* $Id: som.c, v 1.0 05/11/2017 11:51:52  $
 * modify : 11/10/23
 *
 * linux : Affiche la tension en sortie du potentiometre sur le flux stdout et le lcd
 *
 * Copyright (c) 2017 IUT ANNECY Departement GEII/INFO
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 * modification: Yves Marboutin <yves.marboutin@univ-smb.fr>
 */

#include "fctdate.h"
#include "device.h"


#undef  __DEBUG_INFO__
#define __DEBUG_INFO__
//#undef  __DEBUG_INFO__

// magic number: 'l'
#define KLCD162COG_NBR_MAGIQUE 'l'

// commande ioctl
#define LCD_CURSOR_HOME         _IO (KLCD162COG_NBR_MAGIQUE, 0x10)
#define LCD_CLEAR_DISPLAY       _IO (KLCD162COG_NBR_MAGIQUE, 0x11)
#define LCD_CURSOR_DISPLAY      _IO (KLCD162COG_NBR_MAGIQUE, 0x12)
#define LCD_CURSOR_NODISPLAY    _IO (KLCD162COG_NBR_MAGIQUE, 0x13)
#define LCD_CURSOR_BLINKING     _IO (KLCD162COG_NBR_MAGIQUE, 0x14)


/* objet global */
sig_atomic_t sigFlagStop=0;

void gestionnaire_signal(int iNumSignal);

void gestionnaire_signal(int iNumSignal) {

	switch (iNumSignal) {

		case SIGINT:
			sigFlagStop = -1;
			break;

		case SIGTERM:
			sigFlagStop = -1;
			break;
		case SIGALRM:
			break;

		default:
			fprintf(stdout, "GstSig : Nothing to do \n");
			break;
	}
}

int main(int iArgCount, char* apstrArgValue[]){

	/* reservation et initialisation */
	/* reservation et initialisation */
	int iResult = 0;
	int handler = -1;
	float Tmps = -1;
	int TmpsI = -1;
	int AOIx = -1;
	// Volt
	char buffer[SIZESTRV] = {"\0"};
	float ValPot = 0;

	// Heure
	char OutFormat[20] = {"%T\n"};
	char OutDate[SIZESTRDATE];
	char BufferOutDate[SIZESTRDATE];

	
	struct TCharDevice cdevAdcIA0 = {
		.m_strFileIO 	= "/dev/ad7928c0IA0",
		.m_iDescIO 		= -1,
	};
	struct TCharDevice cdevAdcIA1 = {
			.m_strFileIO 	= "/dev/ad7928c0IA1",
			.m_iDescIO 		= -1,
	};
	struct TCharDevice cdevLcd0 = {
			.m_strFileIO 	= "/dev/lcd162cog0",
			.m_iDescIO 		= -1,
	};
	struct TCharDevice DeviceUsed = {
			.m_strFileIO 	= "/dev/lcd162cog0",
			.m_iDescIO 		= -1,
	};
	
	sscanf(apstrArgValue[2],"%i", &AOIx);
	if (AOIx == 1)
	{
		DeviceUsed = cdevAdcIA1;
	}
	else{
		DeviceUsed = cdevAdcIA0;
	}
	

	
	signal(SIGINT, gestionnaire_signal);
	signal(SIGTERM, gestionnaire_signal);
	signal(SIGALRM, gestionnaire_signal);

	//setup()
	DeviceUsed.m_iDescIO = open(DeviceUsed.m_strFileIO, O_RDWR);
	cdevLcd0.m_iDescIO = open(cdevLcd0.m_strFileIO, O_RDWR);
	
	sscanf(apstrArgValue[1],"%f", &Tmps);


	if (Tmps < 1)
	{
		TmpsI = Tmps * 1000000;
		ualarm(TmpsI,TmpsI);
		
	}

	
	


	// traitement 
	while ( sigFlagStop != -1 )
	{
		// Acquisition heure
		DateLocale(OutFormat, OutDate, SIZESTRDATE);
		strcpy(BufferOutDate, OutDate);
		
		// Acquisition Volt
		handler = ReadAdcIA(DeviceUsed.m_iDescIO, buffer, SIZESTRV);
		
		handler = sscanf(buffer, "%f", &ValPot);
		ConvertToVoltage(&ValPot, 5.0);
		
		handler = sprintf(buffer, "IA = %.3f V", ValPot);
		
		// Affichage
		ioctl(cdevLcd0.m_iDescIO, LCD_CURSOR_HOME);
		handler = WriteLcd(cdevLcd0.m_iDescIO, strcat(OutDate, buffer));
		CheckHandler(handler);

		if (Tmps >= 1)
		{
			
			TmpsI = (int) Tmps;
			printf("Valeur recus : %f\n", Tmps);
			printf("A : %i\n", TmpsI);
			alarm(TmpsI);
		}
		

		pause();
		
		
	}
	
	//setdown()
	printf("\nOut While\n");
	ioctl(cdevLcd0.m_iDescIO, LCD_CLEAR_DISPLAY);
	close(DeviceUsed.m_iDescIO);
	close(cdevLcd0.m_iDescIO);
	printf("FIN DU PROGRAMME PAR KILL\n");
	return (iResult);
}
