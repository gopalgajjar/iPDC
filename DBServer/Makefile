 # ----------------------------------------------------------------------------- 
 # Makefile for building DBServer

 # iPDC - Phasor Data Concentrator
 #
 # Copyright (C) 2011 Nitesh Pandit
 # Copyright (C) 2011 Kedar V. Khandeparkar

 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public License
 # as published by the Free Software Foundation; either version 2
 # of the License, or (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program; if not, write to the Free Software
 # Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 #
 # Authors: 
 #		Nitesh Pandit <panditnitesh@gmail.com>
 #		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 #
 # ----------------------------------------------------------------------------- 


SHELL = /bin/sh
prefix = ${HOME}/iPDC
includedir = /usr/src/include

INSTALL = /usr/bin/install -c
INSTALL_PREFIX = $(prefix)

DESKTOP_DIR = ${HOME}/.local/share/applications

FILE1 = DBServer.desktop
FILE2 = runDBServer

CC = /usr/bin/gcc
LIBS = -lpthread -lm
# LDLIBS=-g `mysql_config --cflags --libs` -O
# CFLAGS=-Wall -g `mysql_config --cflags --libs` -std=c99 -O
LDLIBS=-g -O
CFLAGS=-Wall -g -O

SOURCES = \
	dbserver.c \
	recreate.c recreate.h \
	connections.c connections.h \
	parser.c parser.h \
	dallocate.c dallocate.h

OBJECTS =  dbserver.c recreate.o connections.o parser.o dallocate.o 

CLEAN_OBJECTS =  recreate.o connections.o new_pmu_or_pdc.o parser.o dallocate.o cfg.bin

PROGRAM = DBServer

DIRSTRUCT = creatDirStructure.sh
CONVERTFLOAT = convertComtrade.m 

COMPILE = $(CC) 

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(LIBS) -o $@

%.o: %.c
	$(COMPILE) $(CFLAGS) -c -w $<

clean:
	rm -f $(CLEAN_OBJECTS) $(PROGRAM)

install: $(PROGRAM)
	mkdir -p $(INSTALL_PREFIX)/bin/$
	mkdir -p $(INSTALL_PREFIX)/historianData/$
	cp -f $(PROGRAM) $(INSTALL_PREFIX)/bin/$(PROGRAM)
	cp -f $(DIRSTRUCT) $(INSTALL_PREFIX)/bin/$(DIRSTRUCT)
	cp -f $(CONVERTFLOAT) $(INSTALL_PREFIX)/bin/$(CONVERTFLOAT)
	chmod +x $(INSTALL_PREFIX)/bin/$(DIRSTRUCT)
	cp -f $(FILE2) $(INSTALL_PREFIX)/bin/$(FILE2)
	chmod +x $(INSTALL_PREFIX)/bin/$(FILE2)
	cp -f $(FILE1) $(DESKTOP_DIR)/$(FILE1)

uninstall:
	rm -f $(INSTALL_PREFIX)/bin/$(PROGRAM)
	rm -f $(INSTALL_PREFIX)/bin/$(DIRSTRUCT)
	rm -rf $(DESKTOP_DIR)/$(FILE1)
connections.o: connections.c connections.h parser.h global.h
dallocate.o: dallocate.c parser.h dallocate.h global.h
dbserver.o: dbserver.c connections.h parser.h recreate.h
parser.o: parser.c parser.h global.h dallocate.h connections.h
recreate.o: recreate.c parser.h global.h recreate.h
Sample\ DBConnect.o: Sample\ DBConnect.c


