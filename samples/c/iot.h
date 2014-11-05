/*******************************************************************************
* Copyright (c) 2014 IBM Corporation and other Contributors.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* which accompanies this distribution, and is available at
* http://www.eclipse.org/legal/epl-v10.html
*
* Contributors:
*   Jeffrey Dare - Initial Contribution
*******************************************************************************/

/*
 * iot.h
 */

#ifndef IOT_H_
#define IOT_H_

#define MAXBUF               100
#define MSPROXY_URL          "tcp://46.16.189.243:1883"
#define MSPROXY_URL_SSL      "ssl://46.16.189.242:8883"
#define EVENTS_INTERVAL      1
#define CONFIGFILE           "/etc/iotsample-raspberrypi/device.cfg"
#define PUBLISHTOPIC         "iot-2/evt/status/fmt/json"
#define SUBSCRIBEREBOOTTOPIC "iot-2/cmd/reboot/fmt/json"
#define SUBSCRIBEGPIOTOPIC   "iot-2/cmd/gpio/fmt/json"

/* Add this as -DDEVICE_NAME='"whatever"' on the compile line */
#if !defined(DEVICE_NAME)
#define DEVICE_NAME "myPi"
#endif

/* Syslog logging level: Default is INFO-6. Others: ERROR-3, INFO-6, DEBUG-7 */
#define LOGLEVEL 6

/* Configuration */
typedef struct {
   char org[MAXBUF];
   char type[MAXBUF];
   char id[MAXBUF];
   char authmethod[MAXBUF];
   char authtoken[MAXBUF];
   int  isRegistered;
   int  delay;
   int  reboot;
   int  cpu;
   int  gpio;
   int  ds18b20;
   int  camera;
} IOTCONFIG;

extern int connected;

#define DEBUGPRINT printf("%s - %d\n", __FILE__, __LINE__)

#endif /* IOT_H_ */
