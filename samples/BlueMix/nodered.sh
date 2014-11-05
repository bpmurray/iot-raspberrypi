#!/bin/bash

# Set variables
PROJECCT=beespy
GPIO=/usr/local/bin/gpio
URL=http://${PROJECT}.mybluemix.net/photo
CAMERA=/usr/bin/raspistill
CURL=/usr/bin/curl
BASE64=/usr/bin/base64
PICSDIR=/tmp/

# Turn on the LEDs
${GPIO} write 0 1
${GPIO} write 3 1
${GPIO} write 6 1

# Take a pic
${CAMERA} -w 1024 -h 768 -ex night -o ${PICSDIR}latest.jpg 2>>${PICSDIR}error.log

# Turn off LEDS
${GPIO} write 0 0
${GPIO} write 3 0
${GPIO} write 6 0

# Copy to application server over REST API
echo -n `date "+{\"timestamp\":99999,\"pic\":\""` >$(PICSDIR)/curl.data
${BASE64} -w 0 ${PICSDIR}latest.jpg >>$(PICSDIR)/curl.data
echo -n "\"}" >>$(PICSDIR)/curl.data

${CURL} -s -X POST -d @$(PICSDIR)curl.data -H "Content-Type: application/json" ${URL} >/dev/null 2>&1

rm ${PICSDIR}curl.data

