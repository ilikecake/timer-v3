/*
 * sun.h
 *
 *  Created on: Jan 7, 2014
 *      Author: pat
 */

#ifndef SUN_H_
#define SUN_H_

#include "main.h"

void SetLatitude(int16_t LHS, uint16_t RHS);
void SetLongitude(int16_t LHS, uint16_t RHS);

void GetLatitude(int16_t *LHS, uint16_t *RHS);
void GetLongitude(int16_t *LHS, uint16_t *RHS);

//uint32_t ConvertToJD(TimeAndDate* TheTime);

void GetSunriseAndSunsetTime(struct tm* SunriseTime, struct tm* SunsetTime);

#endif /* SUN_H_ */
