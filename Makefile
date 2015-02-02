#
# Makefile to build  XBEE Controller
# Author: Bethany Seeger bethany@seeger.ws
#


#DEBUG = -ggdb3
DEBUG =
WISH = wish
CC = gcc
# INC = 
#CFLAGS  = -Wall `pkg-config --cflags gtk+-2.0` $(DEBUG) $(INC) -D_FILE_OFFSET_BITS=64 \ 
#-D_LARGEFILE_SOURCE $(CUSTOM_FLAGS)
CFLAGS  = -Wall  $(DEBUG) $(INC) -D_FILE_OFFSET_BITS=64 \
-D_LARGEFILE_SOURCE $(CUSTOM_FLAGS)
LDFLAGS = 

#LDLIBS = `pkg-config --libs gtk+-2.0 gthread-2.0` 
LDLIBS = 

OBJS = XBEE_Controller.o

SRCS = $(OBJS:.o=.c)

all: .Depend XBEE_Controller 

XBEE_Controller: XBEE_Controller.o

.Depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS) > .Depend

-include .Depend

clean:
	rm -f $(OBJS) .Depend

distclean: clean
	rm -f XBEE_Controller.o
