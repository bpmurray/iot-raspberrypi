/******************************************************************************
 * Copyright (c) 2014 IBM Corporation and other Contributors.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Brendan Murray - Initial Contribution
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iot.h"
#include "wiringPi.h"

int gpio_get(int pin)
{
   wiringPiSetup();
   pinMode(pin, INPUT);
   return digitalRead(pin);
}

char *gpio_getJSON(IOTCONFIG *pCfg, int pin)
{
   char *pJSON;
   if (!pCfg->gpio)
      return NULL;

   pJSON = malloc(30);
   sprintf(pJSON, "{ \"pin\": %d, \"value\": %d }", pin, gpio_get(pin));
   return pJSON;
}

void gpio_set(int pin, int state)
{
   wiringPiSetup();
   pinMode(pin, OUTPUT);
   digitalWrite(pin, state);
}

