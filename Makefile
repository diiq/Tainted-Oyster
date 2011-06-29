# OK, you should modify these line so that they reflect 
# the directories of glib.h and glibconfig.h
GLIBH_LOCATION=/usr/include/glib-2.0
GLIBCONFIGH_LOCATION=/usr/lib/glib-2.0/include/
CFLAGS=-Wall -lglib-2.0 -I$(GLIBH_LOCATION) -I$(GLIBCONFIGH_LOCATION) -I./tests/ -I./
DEBUG_FLAGS=-g3
PROFILE_FLAGS= -fprofile-arcs -ftest-coverage -pg
TARGET=tainted

all:
	gcc $(CFLAGS) $(TARGET).c  -o $(TARGET)  

debug:
	gcc $(CFLAGS) -o $(TARGET) $(TARGET).c $(DEBUG_FLAGS)

profile:
	gcc $(CFLAGS) $(TARGET).c -o $(TARGET) $(PROFILE_FLAGS) 

test:
	@gcc $(CFLAGS) tests/test_all.c -o testes $(DEBUG_FLAGS) -DGC_DEBUG
	@ ./testes 
#	@rm testes
