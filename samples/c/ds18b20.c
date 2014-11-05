/******************************************************************************
 * Copyright (c) 2014 IBM Corporation and other Contributors.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 ******************************************************************************/

/**
 * Retrieve temperature readings from DS18B20 sensors.
 * <p>
 * This retrieves the values from the sensors that are captured using the
 * 1-wire protocol which is now built into the Raspberry Pi's kernel. All
 * data are simply stored in files named according to the sensor's unique
 * ID, containing the most recently read value. The files are located in
 * the root directory /sys/bus/w1/devices and each device has its files
 * in subdirectories named 28-<ID>  where "<ID>" is the device ID which
 * is composed of 12 hexadecimal digits.
 *
 *
 * @name        ds18b20.c
 * @author      Brendan Murray - brendan_murray at ie dot ibm dot com
 * @version     1.0
 * @since       2014-08-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <time.h>
#include "iot.h"

#define W1PATT "/sys/bus/w1/devices/28-*" /* Pattern to search for sensors */
#define W1NAME "w1_slave"                 /* Name of 1-wire data file */
#define IDLEN  20                         /* Length of sensor ID */
#define MAXSENSORCOUNT 25                 /* Maximum number of sensors */

/* Describes a temperature reading */
typedef struct {
   char   name[IDLEN+1];
   float  temp;
}  TEMPERATURE;


/**
 * @name  ds18b20_getJSON
 *
 * Returns the JSON array of sensor data, each containing the name of the
 * sensor, along with the current temperature reading. 
 *
 * @return String containing JSON representation of the sensor values
 */
char *ds18b20_getJSON(IOTCONFIG *pCfg)
{
   glob_t      globbuf;         /* Used to find the files */
   int         iX, iY;          /* Index variables */
   TEMPERATURE *TempTab = NULL; /* Table of temperature readings */
   char        fPath[50];       /* Buffer for the file name */
   FILE        *fp;             /* File that contains the sensor data */
   char        fBuff[60];       /* IO buffer for reading file */
   char        *pJSON = NULL;   /* JSON buffer */

   /* Check if we are to report */
   if (!pCfg->ds18b20)
      return NULL;

   /* Find the sensor directories */
   glob(W1PATT, GLOB_ONLYDIR, NULL, &globbuf);

   /* Read data from the sensor files */
   if (globbuf.gl_pathc > 0)
   {
      TempTab = calloc(globbuf.gl_pathc+2, sizeof(TEMPERATURE));
      iY = 0;

      for (iX=0; iX<globbuf.gl_pathc; iX++)
      {
          sprintf(fPath,"%s/%s", globbuf.gl_pathv[iX], W1NAME);
          fp = fopen(fPath, "r");
          fgets(fBuff, sizeof(fBuff), fp);
          if (!strncmp(fBuff+strlen(fBuff)-4, "YES", 3))
          {
             strcpy(TempTab[iY].name, globbuf.gl_pathv[iX]+23);
             fgets(fBuff, sizeof(fBuff), fp);
             TempTab[iY++].temp = atoi(fBuff+29) / 1000.0;
          }
          fclose(fp);
      }
   }
   globfree(&globbuf); /* Free memory */


   if (TempTab && iY>0)
   {
      /* We have the table - convert to string */
      pJSON = malloc((iY+2)*60); // Make sure we have space
      pJSON[0] = 0;

      if (pCfg->isRegistered)
      {
         strcpy(pJSON, "\"temperature\" : [ ");

         for (iX=0; iX<iY; iX++)
         {
            strcpy(pJSON+strlen(pJSON), "{ \"date\": 999,");
            sprintf(pJSON+strlen(pJSON), "\"sensor\":\"%s\", \"temp\":%.2f }",
                 TempTab[iX].name, TempTab[iX].temp);
            if (iX < iY-1)
               strcat(pJSON, ", ");
         }
         strcat(pJSON, " ]");
      } else {
         for (iX=0; iX<iY; iX++)
         {
            if (iX)
               strcat(pJSON, ", ");
            sprintf(pJSON+strlen(pJSON), "\"%s\": %.2f",
                    TempTab[iX].name, TempTab[iX].temp);
         }
      }
   }

   /* Clean up */
   free(TempTab);
   return pJSON;
}

