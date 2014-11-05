/*******************************************************************************
* Copyright (c) 2014 IBM Corporation and other Contributors.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* which accompanies this distribution, and is available at
* http://www.eclipse.org/legal/epl-v10.html
*
* Contributors:
*   Amit Mangalvedkar - Initial Contribution
*   Brendan Murray - Modularised and cleaned & move sine generation to here
*******************************************************************************/

/*
 * This function generates the json based on the events from Raspberry Pi
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "iot.h"

#define JSONBLOCKSIZE  10240

/* Incremented every visit */
static int counter = 1;

/* Count number of memory blocks used */
static  int  memBlks = JSONBLOCKSIZE;


/* Function references */
char * cpustat_getJSON();      /* Get JSON for CPU data */
char * ds18b20_getJSON();      /* Get JSON for temperature sensor data */
char * camera_getJSON();       /* Get JSON for camera data */
float  sine_get(float, float, float, float); /* Sample function */
char * CHK_strcat(char *, char *); /* Safe realloc strcat() */

char * generateJSON(IOTCONFIG *pCfg) {
   char *pTmp;
   char *jsonReturned;

   counter++;

   jsonReturned = malloc(memBlks);

   sprintf(jsonReturned, "{ \"d\": { \"myName\":\"%s\"", DEVICE_NAME);

   /* Get the CPU stats - only print if there's something */
   pTmp = cpustat_getJSON(pCfg);
   if (pTmp && strlen(pTmp))
   {
      jsonReturned = CHK_strcat(jsonReturned, ", ");
      jsonReturned = CHK_strcat(jsonReturned, pTmp);
   }

   /* Get the temperature - only print if there's something */
   pTmp = ds18b20_getJSON(pCfg);
   if (pTmp && strlen(pTmp))
   {
      jsonReturned = CHK_strcat(jsonReturned, ", ");
      jsonReturned = CHK_strcat(jsonReturned, pTmp);
      free(pTmp);
   }

   /* Get the camera image - only print if there's something */
   pTmp = camera_getJSON(pCfg);
   if (pTmp && strlen(pTmp))
   {
      jsonReturned = CHK_strcat(jsonReturned, ", ");
      jsonReturned = CHK_strcat(jsonReturned, pTmp);
      free(pTmp);
   }

   /* Close the JSON */
   jsonReturned = CHK_strcat(jsonReturned, "} }");
   return jsonReturned;
}


/* This function generates the sine value based on the interval specified
 * and the duration
 */
float sine_get(float minValue, float maxValue, float duration, float count) {
   float sineValue;
   sineValue = sin(2.0 * M_PI * count / duration) * (maxValue - minValue) / 2.0;
   return sineValue;
}



/* This does a strcat, but only after it checks that there's enough
 * room, calling realloc to expand the target string if required.
 */
char * CHK_strcat(char *s1, char *s2)
{
   if (strlen(s1)+strlen(s2) >= memBlks)
   {
      memBlks += strlen(s2) + JSONBLOCKSIZE;
      s1 = realloc(s1, memBlks);
   }
   return strcat(s1, s2);
}


