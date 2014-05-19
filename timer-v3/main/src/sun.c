/*
 * sun.c
 *
 *  Created on: Jan 7, 2014
 *      Author: pat
 */

//Based on the sunrise and sunset algorithm from http://williams.best.vwh.net/sunrise_sunset_algorithm.htm

#include "main.h"
#include <time.h>

/**Sun's zenith for sunrise/sunset
 *
 *  offical      = 90 degrees 50'
 *  civil        = 96 degrees
 *  nautical     = 102 degrees
 *  astronomical = 108 degrees
 */
#define SUN_ZENITH		90.83333
#define DEG2RAD(x) (3.141592*(x)/180)
#define RAD2DEG(x) (180*(x)/3.141592)

//double Latitude;
//double Longitude;		//Longitude east (for west, use negative numbers)

//int16_t LatitudeLHS;
//uint16_t LatitudeRHS;
//int16_t LongitudeLHS;
//uint16_t LongitudeRHS;

/**Update the Latitude value stored in EEPROM. Latitude is positive north.
 *
 *	LHS:	The integer portion of the latitude (signed)
 * 	RHS: 	The decimal portion of the latitude (unsigned).
 *
 * Note: the RHS can accommodate four decimal places. Ex: To enter 43.0833 N, LHS is 43, RHS is 0833
 */
void SetLatitude(int16_t LHS, uint16_t RHS)
{
	if( EEPROM_Write(EEPROM_ADDRESS_LAT_LHS_MSB, &LHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
	if( EEPROM_Write(EEPROM_ADDRESS_LAT_RHS_MSB, &RHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
	return;
}

/**Update the longitude value stored in EEPROM. Longitude is positive east.
 *
 * 	LHS: 	The integer portion of the longitude (signed)
 * 	RHS: 	The decimal portion of the longitude (unsigned)
 *
 * Note: the RHS can accommodate four decimal places. Ex: To enter 89.3833 W, LHS is -89, RHS is 3833
 */
void SetLongitude(int16_t LHS, uint16_t RHS)
{
	//Save new value in EEPROM
	if( EEPROM_Write(EEPROM_ADDRESS_LONG_LHS_MSB, &LHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	//Save new value in EEPROM
	if( EEPROM_Write(EEPROM_ADDRESS_LONG_RHS_MSB, &RHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}





	/*if(RHS > 9999)
	{
		RHS = 0;
	}
	if(LHS < 0)
	{
		Longitude = (double)LHS - ((double)RHS)/10000.0;
	}
	else
	{
		Longitude = (double)LHS + ((double)RHS)/10000.0;
	}*/
	return;
}

/** Read Latitude from EEPROM */
void GetLatitude(int16_t *LHS, uint16_t *RHS)
{
	if( EEPROM_Read(EEPROM_ADDRESS_LAT_LHS_MSB, LHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	//Save new value in EEPROM
	if( EEPROM_Read(EEPROM_ADDRESS_LAT_RHS_MSB, RHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
	return;
}

/** Read Longitude from EEPROM */
void GetLongitude(int16_t *LHS, uint16_t *RHS)
{
	if( EEPROM_Read(EEPROM_ADDRESS_LONG_LHS_MSB, LHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}

	//Save new value in EEPROM
	if( EEPROM_Read(EEPROM_ADDRESS_LONG_RHS_MSB, RHS, 2 ) !=0)
	{
		//Failed to write to EEPROM
		App_Die(8);
	}
	return;
}





//Calculate the sunrise and sunset time from the date, latitude and longitude
//Sunrise time should have the day, month, and year filled.
//TODO: See if I can minimize the number of variables needed here
//NOTE: This function returns the sunrise and sunset time in UT, the local offset from UT must be applied elsewhere
//void GetSunriseAndSunsetTime(TimeAndDate* SunriseTime, TimeAndDate* SunsetTime)
void GetSunriseAndSunsetTime(struct tm* SunriseTime, struct tm* SunsetTime)
{
	int16_t LHS;

	uint16_t N1;
	uint16_t N2;
	uint16_t N3;
	uint16_t N;

	double Latitude;
	double Longitude;

	double lngHour;
	double t;
	double M;
	double L;
	double RA;
	double Lquad;
	double RAquad;
	double sinDec;
	double cosDec;
	double cosH;
	double H;
	double bT;
	double UT;

	uint16_t WorkingMonth;
	uint16_t WorkingYear;

	//Get latitude and longitude from EEPROM
	//Use N1 as a temporary variable
	GetLatitude(&LHS, &N1);
	if(N1 > 9999)
	{
		N1 = 0;
	}
	if(LHS < 0)
	{
		Latitude = (double)LHS - ((double)N1)/10000.0;
	}
	else
	{
		Latitude = (double)LHS + ((double)N1)/10000.0;
	}

	GetLongitude(&LHS, &N1);
	if(N1 > 9999)
	{
		N1 = 0;
	}
	if(LHS < 0)
	{
		Longitude = (double)LHS - ((double)N1)/10000.0;
	}
	else
	{
		Longitude = (double)LHS + ((double)N1)/10000.0;
	}



	//Convert the month and year definitions from the C standard to what the function expects.
	WorkingMonth = SunriseTime->tm_mon+1;
	WorkingYear = SunriseTime->tm_year+1900;
	//double LT;

	//printf("Lat: %f\r\n", Latitude);
	//printf("Long: %f\r\n", Longitude);

	N1 = ((275*(WorkingMonth))/9);
	N2 = (WorkingMonth+9)/12;
	N3 = 1 + ((WorkingYear - 4*(WorkingYear/4) + 2) / 3);
	N = N1 - (N2*N3) + SunriseTime->tm_mday - 30;

	lngHour = Longitude / 15;

	//Calculation for sunrise
	t = N + ((6-lngHour)/24);
	M = (.9856*t) - 3.289;

	L = M + (1.916*sin(DEG2RAD(M))) + (.02*sin(DEG2RAD(2*M))) + 282.634;
	if(L > 360)
	{
		L = L-360;
	}
	else if(L < 0)
	{
		L += 360;
	}

	RA = RAD2DEG(atan(.91764*tan(DEG2RAD(L))));
	if(RA > 360)
	{
		RA = RA-360;
	}
	else if(RA < 0)
	{
		RA += 360;
	}

	Lquad = floor(L/90)*90;
	RAquad = floor(RA/90)*90;
	RA = RA+(Lquad - RAquad);

	RA = RA/15;

	sinDec = .39782*sin(DEG2RAD(L));
	cosDec = cos(asin(sinDec));

	cosH = (cos(DEG2RAD(SUN_ZENITH)) - (sinDec*sin(DEG2RAD(Latitude))))/(cosDec*cos(DEG2RAD(Latitude)));

	//cosH > 1 means the sun never rises at this location on this date
	//cosH < -1 means the sun never sets at this location on this date
	//TODO: deal with these later

	H = 360-RAD2DEG(acos(cosH));
	H = H/15;

	bT = H + RA - (.06571*t) - 6.622;
	UT = bT - lngHour;
	if(UT > 24)
	{
		UT -= 24;
	}
	else if(UT < 0)
	{
		UT += 24;
	}

	//LT = UT-6;			//CST
	//printf("LT: %f\r\n", LT);
	//printf("UT1: %f\r\n", UT);

	SunriseTime->tm_hour = (uint8_t)(floor(UT));
	SunriseTime->tm_min = (uint8_t)((UT-floor(UT))*60);
	SunriseTime->tm_sec = 0;		//We could probably get this from the calculation, but it should not be needed.

	//Calculation for sunset
	t = N + ((18-lngHour)/24);
	M = (.9856*t) - 3.289;

	L = M + (1.916*sin(DEG2RAD(M))) + (.02*sin(DEG2RAD(2*M))) + 282.634;
	if(L > 360)
	{
		L = L-360;
	}
	else if(L < 0)
	{
		L += 360;
	}

	RA = RAD2DEG(atan(.91764*tan(DEG2RAD(L))));
	if(RA > 360)
	{
		RA = RA-360;
	}
	else if(RA < 0)
	{
		RA += 360;
	}

	Lquad = floor(L/90)*90;
	RAquad = floor(RA/90)*90;
	RA = RA+(Lquad - RAquad);

	RA = RA/15;

	sinDec = .39782*sin(DEG2RAD(L));
	cosDec = cos(asin(sinDec));

	cosH = (cos(DEG2RAD(SUN_ZENITH)) - (sinDec*sin(DEG2RAD(Latitude))))/(cosDec*cos(DEG2RAD(Latitude)));

	//cosH > 1 means the sun never rises at this location on this date
	//cosH < -1 means the sun never sets at this location on this date
	//TODO: deal with these later

	H = RAD2DEG(acos(cosH));
	H = H/15;

	bT = H + RA - (.06571*t) - 6.622;
	UT = bT - lngHour;
	if(UT > 24)
	{
		UT -= 24;
	}
	else if(UT < 0)
	{
		UT += 24;
	}

	//LT = UT-6;			//CST
	//printf("LT: %f\r\n", LT);

	//printf("UT2: %f\r\n", UT);

	SunsetTime->tm_mday		= SunriseTime->tm_mday;
	SunsetTime->tm_mon	= SunriseTime->tm_mon;
	SunsetTime->tm_year	= SunriseTime->tm_year;
	SunsetTime->tm_hour	= (uint8_t)(floor(UT));
	SunsetTime->tm_min		= (uint8_t)((UT-floor(UT))*60);
	SunsetTime->tm_sec		= 0;		//We could probably get this from the calculation, but it should not be needed.
	return;
}

/*uint32_t ConvertToJD(TimeAndDate* TheTime)
{
	uint32_t JD;
	uint32_t a;
	uint32_t y;
	uint32_t m;

	a = (uint32_t)((14-TheTime->month)/12);
	y = (uint32_t)(TheTime->year) + 4800ul - a;
	m = TheTime->month + 12*a - 3;

	JD = (uint32_t)(TheTime->day) + (((153*m)+2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
	return JD;
}*/
