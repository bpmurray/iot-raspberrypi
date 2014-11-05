/******************************************************************************
 * Copyright (c) 2014 IBM Corporation and other Contributors.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 ******************************************************************************/

/**
 * Capture an image from the camera
 * <p>
 * This captures an image and returns it as a BASE64 blob in the json data
 * which are then included in the MQTT payload to the BlueMix IOT integration.
 *
 * @name        camera.c
 * @author      Brendan Murray - brendan_murray at ie dot ibm dot com
 * @version     1.0
 * @since       2014-08-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <glob.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include "iot.h"

char *base64Encode(const unsigned char *, size_t, size_t *);

#define RASPISTILL "/usr/bin/raspistill -w 480 -h 320 --colfx 128:128 -ex night -e jpg -o /tmp/iotpic.jpg"
#define RASPIFILE  "/tmp/iotpic.jpg"
#define PICSIZE    50000


/**
 * @name  camera_getJSON
 *
 * Returns the JSON encapsulation the image data.
 *
 * @return String containing JSON representation of the image
 */
char *camera_getJSON(IOTCONFIG *pCfg)
{
   int    fp;              /* File that contains the camera data */
   unsigned char *pBuff;   /* Image buffer */
   char   *pBase64;        /* BASE64 buffer */
   char   *pJSON;          /* JSON buffer */
   size_t bytesRead;       /* Size of input data */
   size_t bytesBase64;     /* Size of converted data */
   struct stat statBuf;    /* Status structure */

   /* Check if we are to report */
   if (!pCfg->camera)
      return NULL;

   /* Capture the image */
   system(RASPISTILL);
   fp = open(RASPIFILE, O_RDONLY);
   if (fp == -1)
   {
      syslog(LOG_ERR, "Cannot capture the image");
      return NULL;
   }

   fstat(fp, &statBuf);
   pBuff = malloc(statBuf.st_size + 20);
   if (!pBuff)
   {
      return NULL;
   }
   memset(pBuff, 0, statBuf.st_size+20);
   bytesRead = read(fp, pBuff, statBuf.st_size+20);
   close(fp);
   remove(RASPIFILE);

   /* Convert to BASE64 */
   pBase64 = base64Encode(pBuff, bytesRead, &bytesBase64);
   if (!pBase64)
   {
      free(pBuff);
      return NULL;
   }

   pJSON = malloc(bytesBase64+32);
   if (!pJSON)
   {
      free(pBuff);
      free(pBase64);
      return NULL;
   }

   sprintf(pJSON, "\"image\": \"%.*s\"", bytesBase64, pBase64);

   /* Clean up */
   free(pBuff);
   free(pBase64);
   return pJSON;
}


/**
 * This takes a data blob and encodes using base64
 *
 * @name  base64Encode
 *
 * @param  data   - Data to be encoded, typically binary
 * @param  inLen  - Length of the input data
 * @param  outLen - Pointer to the field to receive the output length
 *
 * @return pointer to potentially reallocated memory block
 */
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static int mod_table[] = {0, 2, 1};

char *base64Encode(const unsigned char *data, size_t inLen, size_t *outLen)
{
   int  iX, iY;
   char *encData;

   *outLen  = 4 * ((inLen + 2) / 3);
   encData = malloc((*outLen) + 32);

   if (encData)
   {
      memset(encData, 0, (*outLen) + 32);

      for (iX = 0, iY = 0; iX < inLen;) {

         uint32_t octet_a = iX < inLen ? (unsigned char)data[iX++] : 0;
         uint32_t octet_b = iX < inLen ? (unsigned char)data[iX++] : 0;
         uint32_t octet_c = iX < inLen ? (unsigned char)data[iX++] : 0;
 
         uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

         encData[iY++] = encoding_table[(triple >> 3 * 6) & 0x3F];
         encData[iY++] = encoding_table[(triple >> 2 * 6) & 0x3F];
         encData[iY++] = encoding_table[(triple >> 1 * 6) & 0x3F];
         encData[iY++] = encoding_table[(triple >> 0 * 6) & 0x3F];
      }
   }

   for (iX = 0; iX < mod_table[inLen % 3]; iX++)
   {
      encData[*outLen - 1 - iX] = '=';
   }

   return encData;
}

