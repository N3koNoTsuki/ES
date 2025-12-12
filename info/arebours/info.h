/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * linux : Affiche l'heure HH:MN sur le flux stdout
 *
 * Copyright (c) 2017 IUT ANNNECY Departement GEII/ESE
 * Author: Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>
 */

#if !defined __INFO_H__
#define      __INFO_H__

#define SIZESTRDATE 20

struct TrgbwBrightNess {
   char m_cRed;
   char m_cGreen;
   char m_cBlue;
   char m_cWhite;
};

union TLedBrightNess {
	struct TrgbwBrightNess m_rgbwBrightNess;
	char m_a4cRGBW[4];
	int m_iWBGR;
};



struct TCharDevice {
	char 	m_strFileIO[60];
	int	m_iDescIO;
};


#endif // #if !defined __INFO_H__

