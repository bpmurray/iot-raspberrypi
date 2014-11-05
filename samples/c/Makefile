CP=/bin/cp
MKDIR=/bin/mkdir -p
INSTALL=/usr/bin/install
LN=/bin/ln -s
STRIP=/usr/bin/strip

VERSION=1.0-1-BPM
ARCH=armhf

DIR=$(DESTDIR)/opt/iot
LIBDIR=$(DESTDIR)/usr/lib
CFGDIR=$(DESTDIR)/etc/iotsample-raspberrypi
COMPILELIBDIR=./lib
MQTTLIB=paho-mqtt3as

DEBUG=
INCLUDES=-I/usr/local/include -I.
LIBDIRS=-L/usr/local/lib -L$(LIBDIR) -L$(COMPILELIBDIR)
LIBS=-l$(MQTTLIB) -lwiringPi -lpthread -lssl -lm
CFLAGS=-Wall -DDEVICE_NAME=\"bees\" $(DEBUG) -D DEBUG
IOTSOURCES=camera.c cpustat.c ds18b20.c gpio.c iotmain.c jsonator.c
SOURCES=mac.c mqttPublisher.c jsonReader.c cJSON.c $(IOTSOURCES)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=iot

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o $@ $(LIBS) $(LIBDIRS) 
	$(STRIP) $@

.c.o :
	$(CC) $(CFLAGS) -c $< $(DEBUG) $(CFLAGS) $(INCLUDES)

mqttPublisher.c : MQTTAsync.h  MQTTClientPersistence.h  
	touch $@

$(IOTSOURCES): iot.h
	touch $@

clean:
	rm -f iot $(OBJECTS) ../iot_$(VERSION)_$(ARCH).*

deb:
	debuild -b -us -uc
	
# Remember that the following 2 rules need to be run as su, i.e. calling
#    sudo make install
# or
#    sudo service iot stop
#    sudo make update
#    sudo service iot start
install: $(EXECUTABLE)
	mkdir -p $(LIBDIR)
	$(MKDIR) $(DIR)
	$(MKDIR) $(CFGDIR)
	$(INSTALL) iot $(DIR)/iot
	$(INSTALL) iotGetDeviceID.sh $(DIR)/iotGetDeviceID.sh
	$(CP) README.md $(DIR)/README
	$(CP) IoTFoundation.pem $(DIR)/IoTFoundation.pem
	$(CP) $(COMPILELIBDIR)/libpaho-mqtt3a.so.1.0 $(LIBDIR)
	$(CP) $(COMPILELIBDIR)/libpaho-mqtt3as.so.1.0 $(LIBDIR)
	cd $(LIBDIR) && $(LN) ./libpaho-mqtt3a.so.1.0  ./libpaho-mqtt3a.so.1
	cd $(LIBDIR) && $(LN) ./libpaho-mqtt3as.so.1.0 ./libpaho-mqtt3as.so.1
	cd $(LIBDIR) && $(LN) ./libpaho-mqtt3a.so.1    ./libpaho-mqtt3a.so
	cd $(LIBDIR) && $(LN) ./libpaho-mqtt3as.so.1   ./libpaho-mqtt3as.so

update: $(EXECUTABLE)
	$(INSTALL) $(EXECUTABLE) $(DIR)/$(EXECUTABLE)
