/*******************************************************************************
* Copyright (c) 2014 IBM Corporation and other Contributors.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* which accompanies this distribution, and is available at
* http://www.eclipse.org/legal/epl-v10.html
*
*******************************************************************************/

/**
 * Retrieve CPU metrics
 * <p>
 * This retrieves the values associated with various aspects of the CPU.
 * Specifically it captures the CPU temperature and load, along with the
 * current date and time (ass UTC values)
 *
 * @name        cpustat.c
 * @author      Jeffrey Dare - Initial Contribution
 *              Brendan Murray - cleaned & tweaked
 * @version     1.0.2
 * @since       2014-08-30
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "iot.h"

int PATHSIZE = 255;
int SIZE = 8;
char cputemploc[255] = "/sys/class/thermal/thermal_zone0/temp";
char cpuloadloc[255] = "/proc/loadavg";


/**
 * @name  cpuTemp_get
 *
 * @return the current CPU temperature
 */
float cpuTemp_get() {
	FILE * cputemp = NULL;
	char buffer [SIZE];
	long tempinmillic;
	float tempinc;

	memset(buffer, 0, sizeof(buffer));
	cputemp = fopen(cputemploc, "r");

	fgets(buffer, SIZE, cputemp);

	tempinmillic = atol(buffer);
	tempinc = tempinmillic * 1.0 / 1000.0;

	fclose(cputemp);
	return tempinc;
}


/**
 * @name  cpuLoad_get
 *
 * @return the current processor load
 */
float cpuLoad_get() {
	
	FILE *f1;
        float load1,load5,load15;

        f1 = fopen(cpuloadloc, "r");
        fscanf(f1, "%f\t%f\t%f\t", &load1, &load5, &load15 );
	fclose(f1);
        return (load1);

}


/**
 * @name  cpuDate_get
 *
 * @return the current system date as YYYY-MM-DD
 */
char *cpuDate_get()
{
   static char strDate[12];
   time_t now = time(NULL);
   struct tm *gmt = gmtime(&now);

   strftime(strDate, sizeof(strDate), "%Y-%m-%d", gmt);
   return strDate;
}



/**
 * @name  cpuTime_get
 *
 * @return the current system time as UTC HH:MM:SS
 */
char *cpuTime_get()
{
   static char strTime[10];
   time_t now = time(NULL);
   struct tm *gmt = gmtime(&now);

   strftime(strTime, sizeof(strTime), "%T", gmt);
   return strTime;
}



/**
 * @name  cpustat_getJSON
 *
 * @return the CPU stats as a JSON string
 */
char *cpustat_getJSON(IOTCONFIG *pCfg)
{
   static char pJSON[70];

   /* Check if we are to report */
   if (!pCfg->cpu)
      return NULL;

   /*sprintf(pJSON, " \"cputemp\": %.2f, \"cpuload\": %.2f, \"date\": \"%s\", \"time\": \"%s\"",
           cpuTemp_get(), cpuLoad_get(), cpuDate_get(), cpuTime_get());*/
   sprintf(pJSON, " \"cputemp\": %.2f, \"cpuload\": %.2f",
           cpuTemp_get(), cpuLoad_get());
   return pJSON;
}
