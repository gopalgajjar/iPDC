 # ----------------------------------------------------------------------------- 
 # Makefile for building iPDC

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
 #		Gopal Gajjar <gopalgajjar@ieee.org>
 #
 # ----------------------------------------------------------------------------- 


SHELL = /bin/sh
prefix = /usr/local
includedir = /usr/src/include

INSTALL = /usr/bin/install -c
#INSTALL_PREFIX = /usr/local/
INSTALL_PREFIX = ${HOME}/iPDC
#CREATE_DIR = /usr/local/share/
CREATE_DIR = $(INSTALL_PREFIX)/share/
#DESKTOP_DIR = /usr/share/applications
#DESKTOP_DIR = $(INSTALL_PREFIX)/desktop
DESKTOP_DIR = ${HOME}/.local/share/applications
ICON_DIR = ${HOME}/.local/share/icons

FILE1 = iPDC.xml
FILE2 = ipdcLogo.png
FILE3 = PSLab.PNG
FILE4 = ipdcLogo1.png
FILE5 = logo.png
FILE6 = iPDC.desktop
FILE7 = runiPDC

CC = gcc
LIBS = -lpthread
LDLIBS=-g `pkg-config --cflags --libs gtk+-2.0`
CFLAGS=-g `pkg-config --cflags --cflags gtk+-2.0`

SOURCES = \
	ipdc.c \
	recreate.c recreate.h \
	connections.c connections.h \
	new_pmu_or_pdc.c new_pmu_or_pdc.h \
	align_sort.c align_sort.h \
	parser.c parser.h \
	dallocate.c dallocate.h \
	ipdcGui.c ipdcGui.h \
	global.h 

OBJECTS =  ipdc.c recreate.o connections.o new_pmu_or_pdc.o parser.o dallocate.o align_sort.o ipdcGui.o 

CLEAN_OBJECTS =  recreate.o connections.o new_pmu_or_pdc.o parser.o dallocate.o align_sort.o ipdcGui.o iPDC.xml

PROGRAM = iPDC

COMPILE = $(CC) 

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(LIBS) -o $@ 

%.o: %.c
	$(COMPILE) $(CFLAGS) -c $<

install: $(PROGRAM)
	gtk-builder-convert iPDC.glade iPDC.xml
	mkdir -p $(INSTALL_PREFIX)/iPDC/temp
	mkdir -p $(CREATE_DIR)iPDC/
	cp -f $(FILE1) $(CREATE_DIR)iPDC/$(FILE1)
	cp -f images/$(FILE2) $(CREATE_DIR)iPDC/$(FILE2)
	cp -f images/$(FILE3) $(CREATE_DIR)iPDC/$(FILE3)
	cp -f images/$(FILE4) $(CREATE_DIR)iPDC/$(FILE4)
	cp -f images/$(FILE5) $(CREATE_DIR)iPDC/$(FILE5)
	cp -f images/$(FILE5) $(ICON_DIR)/$(FILE2)
	cp -f images/$(FILE6) $(DESKTOP_DIR)/$(FILE6)
	cp $(PROGRAM) $(INSTALL_PREFIX)/bin/$(PROGRAM)
	cp $(FILE7) $(INSTALL_PREFIX)/bin/$(FILE7)
	chmod +x $(INSTALL_PREFIX)/bin/$(FILE7)
#	rm -f $(CLEAN_OBJECTS) $(PROGRAM)

uninstall:
	rm -f $(INSTALL_PREFIX)/bin/$(PROGRAM)
	rm -f $(INSTALL_PREFIX)/bin/$(FILE7)
	rm -rf $(CREATE_DIR)iPDC$
	rm -rf $(ICON_DIR)/$(FILE2)
	rm -rf $(DESKTOP_DIR)/$(FILE6)
	rm -f $(CLEAN_OBJECTS) $(PROGRAM)

clean:
	rm -f $(CLEAN_OBJECTS) $(PROGRAM)
