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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <syslog.h>
#include <ctype.h>
#include "iot.h"
#include "MQTTAsync.h"


#define CONFIGFILE           "/etc/iotsample-raspberrypi/device.cfg"
#define PUBLISHTOPIC         "iot-2/evt/status/fmt/json"
#define SUBSCRIBEREBOOTTOPIC "iot-2/cmd/reboot/fmt/json"
#define SUBSCRIBEGPIOTOPIC   "iot-2/cmd/gpio/fmt/json"


/* Global variables */
char clientId[MAXBUF]; /* Client ID */
int isRegistered = 0;  /* Running in registered mode or quickstart mode? */
MQTTAsync client;      /* MQTT client  */

/* function references */
int  get_config(char *, IOTCONFIG *);
void getClientId(IOTCONFIG *, char *);
void sig_handler(int);
int  reconnect_delay(int);
char *generateJSON(IOTCONFIG *);
void publishMessage(char *, char *, char *);


/* External functions */
extern char *getmac(char *);

extern int init_mqtt_connection(MQTTAsync *, char *, int, char *,
                                                          char *, char *);
extern int publishMQTTMessage(MQTTAsync *, char *, char *);
extern int subscribe(MQTTAsync *, char *);
extern int disconnect_mqtt_client(MQTTAsync *);
extern int reconnect(MQTTAsync *, int, char *, char *);

int main(int argc, char **argv) {

   char* json;

   int sleepTimeout;
   IOTCONFIG configstr;

   char *passwd;
   char *username;
   char msproxyUrl[MAXBUF];

   //setup the syslog logging
   setlogmask(LOG_UPTO(LOGLEVEL));
   openlog("iot", LOG_PID | LOG_CONS, LOG_USER);
   syslog(LOG_INFO, "IOT service started");

   // register the signal handler for USR1-user defined signal 1
   if (signal(SIGUSR1, sig_handler) == SIG_ERR)
      syslog(LOG_CRIT, "Not able to register the signal handler\n");
   if (signal(SIGINT, sig_handler) == SIG_ERR)
      syslog(LOG_CRIT, "Not able to register the signal handler\n");

   //read the config file, to decide whether to goto quickstart or registered mode of operation
   isRegistered = get_config(CONFIGFILE, &configstr);

   if (isRegistered) {
      syslog(LOG_INFO, "Running in Registered mode\n");
      sprintf(msproxyUrl, "ssl://%s.messaging.internetofthings.ibmcloud.com:8883", configstr.org);
      if(strcmp(configstr.authmethod ,"token") != 0) {
         syslog(LOG_ERR, "Detected that auth-method is not token. Currently other authentication mechanisms are not supported, IoT process will exit.");
         syslog(LOG_INFO, "**** IoT Raspberry Pi Sample has ended ****");
         closelog();
         exit(1);
      } else {
         username = "use-token-auth";
         passwd = configstr.authtoken;
      }
   } else {
      syslog(LOG_INFO, "Running in Quickstart mode\n");
      strcpy(msproxyUrl,"tcp://quickstart.messaging.internetofthings.ibmcloud.com:1883");
   }

   // read the events
   char* mac_address = getmac("eth0");
   getClientId(&configstr, mac_address);
   // the timeout between the connection retry
   int connDelayTimeout = 1;   // default sleep for 1 sec
   int retryAttempt = 0;

   // initialize the MQTT connection
   init_mqtt_connection(&client, msproxyUrl, isRegistered, clientId, username, passwd);
   // Wait till we get a successful connection to IoT MQTT server
   while (!MQTTAsync_isConnected(client)) {
      connDelayTimeout = 1; // add extra delay(3,60,600) only when reconnecting
      if (connected == -1) {
         connDelayTimeout = reconnect_delay(++retryAttempt);   //Try to reconnect after the retry delay
         syslog(LOG_ERR,
               "Failed connection attempt #%d. Will try to reconnect "
               "in %d seconds\n", retryAttempt, connDelayTimeout);
         connected = 0;
         init_mqtt_connection(&client, msproxyUrl, isRegistered, clientId, username,
               passwd);
      }
      fflush(stdout);
      sleep(connDelayTimeout);
   }

   // resetting the counters
   connDelayTimeout = 1;
   retryAttempt = 0;

   sleepTimeout = EVENTS_INTERVAL;

   //subscribe for commands - only on registered mode
   if (isRegistered) {
      subscribe(&client, SUBSCRIBEREBOOTTOPIC);
      subscribe(&client, SUBSCRIBEGPIOTOPIC);
   }
   while (1) {
      json = generateJSON(&configstr);
      publishMessage(json, username, passwd);
      free(json);
      fflush(stdout);
      sleep(sleepTimeout);
      if (configstr.delay)
         sleep(configstr.delay);
   }

   return 0;
}

//This generates the clientID based on the tenant_prefix and mac_address(external Id)
void getClientId(IOTCONFIG *configstr, char* mac_address) {

   char *orgId;
   char *typeId;
   char *deviceId;

   if (isRegistered) {

      orgId = configstr->org;
      typeId = configstr->type;
      deviceId = configstr->id;

   } else {

      orgId = "quickstart";
      typeId = "iotsample-raspberrypi";
      deviceId = mac_address;

   }
   sprintf(clientId, "d:%s:%s:%s", orgId, typeId, deviceId);
}

// Signal handler to handle when the user tries to kill this process. Try to close down gracefully
void sig_handler(int signo) {
   syslog(LOG_INFO, "Terminating the IoT process gracefully.\n");

   int res = disconnect_mqtt_client(&client);

   syslog(LOG_INFO, "Disconnect finished with result code : %d\n", res);
   closelog();
   exit(1);
}



/* Reconnect delay time 
 * depends on the number of failed attempts
 */
int reconnect_delay(int i) {
   if (i < 10) {
      return 3; // first 10 attempts try within 3 seconds
   }
   if (i < 20)
      return 60; // next 10 attempts retry after every 1 minute

   return 600;   // after 20 attempts, retry every 10 minutes
}




/* Trimming string */
char *trim(char *pStr) {
   if (pStr)
   {
      char *pEnd = pStr + strlen(pStr) - 1;

      if (pStr) {
        while(*pStr && isspace(*pStr))
           pStr++;
        if (*pStr == '#')
           *pStr = 0;

        pEnd = pStr + strlen(pStr) - 1;
        while (pEnd>pStr && isspace(*pEnd))
           *pEnd-- = 0;
      }
   }

   return pStr;
}


// This is the function to read the config from the device.cfg file
int get_config(char * filename, IOTCONFIG * pConfig) {

   FILE *fProp = fopen(filename, "r");
   char line[256];
   int  linenum = 0;

   /* Set default values */
   pConfig->delay = pConfig->reboot  = pConfig->gpio
                  = pConfig->camera  = pConfig->isRegistered = 0;
   pConfig->cpu   = pConfig->ds18b20 = 1;

   if (!fProp) { /* No file - must be quickstart */
      syslog(LOG_INFO, "Config file not found. Going to Quickstart mode\n");
      return 0;
   }

   /* Now read the data */
   while (fgets(line, 256, fProp)) {
      char *key;
      char *value;
      int  numval;
      char *pStr = trim(strtok(line, "#")); /* Strip comments */
      linenum++;

      if (pStr && *pStr != '#' && strlen(pStr))
      {
         key    = trim(strtok(line, "="));
         value  = trim(strtok(NULL, "="));
         if (key && strlen(key) && value && strlen(value))
         {
            numval = atoi(value);

            if (strcmp(key, "org") == 0)
               strncpy(pConfig->org, value, MAXBUF);
            else if (strcmp(key, "type") == 0)
               strncpy(pConfig->type, value, MAXBUF);
            else if (strcmp(key, "id") == 0)
               strncpy(pConfig->id, value, MAXBUF);
            else if (strcmp(key, "auth-token") == 0)
               strncpy(pConfig->authtoken, value, MAXBUF);
            else if (strcmp(key, "auth-method") == 0)
               strncpy(pConfig->authmethod, value, MAXBUF);
            else if (strcmp(key, "delay") == 0)
               pConfig->delay = numval;

            /* The following are boolean, i.e. 0 or 1 */
            else if (strcmp(key, "reboot") == 0)
               pConfig->reboot = numval ? 1 : 0;
            else if (strcmp(key, "gpio") == 0)
               pConfig->gpio = numval ? 1 : 0;
            else if (strcmp(key, "ds18b20") == 0)
               pConfig->ds18b20 = numval ? 1 : 0;
            else if (strcmp(key, "camera") == 0)
               pConfig->camera = numval ? 1 : 0;
            else if (strcmp(key, "cpu") == 0)
               pConfig->cpu = numval ? 1 : 0;
         }
      }
   }


   fclose(fProp);

   pConfig->isRegistered = 1;

   return pConfig->isRegistered;
}

void publishMessage(char *json, char *username, char *password)
{
   int connDelayTimeout = 1;
   int retryAttempt = 0;

   int res = publishMQTTMessage(&client, PUBLISHTOPIC, json);

   syslog(LOG_DEBUG, "Posted the message with result code = %d\n", res);
   if (res == -3) {
      //update the connected to connection failed
      connected = -1;
      while (!MQTTAsync_isConnected(client)) {
         if (connected == -1) {
            connDelayTimeout = reconnect_delay(++retryAttempt); //Try to reconnect after the retry delay
            syslog(LOG_ERR, "Failed connection attempt #%d. "
                     "Will try to reconnect in %d "
                     "seconds\n", retryAttempt, connDelayTimeout);
            sleep(connDelayTimeout);
            connected = 0;
            reconnect(&client, isRegistered, username,password);
         }
         fflush(stdout);
         sleep(1);
      }
      // resetting the counters
      connDelayTimeout = 1;
      retryAttempt = 0;
   }
   fflush(stdout);
}
