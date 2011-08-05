# OK, you should modify these line so that they reflect 
# the directories of glib.h and glibconfig.h
GLIBFLAGS=`pkg-config --cflags --libs  glib-2.0`
CFLAGS=-Wall $(GLIBFLAGS) -Itests -Iheaders -Isrc
DEBUG_FLAGS=-g3
PROFILE_FLAGS= -fprofile-arcs -pg
TARGET=tainted
MODULES=src/*.c

all:
	gcc $(CFLAGS) $(TARGET).c $(MODULES)  -o $(TARGET)  

debug:
	gcc $(CFLAGS) $(TARGET).c $(MODULES)  -o $(TARGET) $(DEBUG_FLAGS)

profile:
	gcc $(CFLAGS) $(TARGET).c $(MODULES)  -o $(TARGET) $(PROFILE_FLAGS) 

test:
	@gcc $(CFLAGS) tests/test_all.c $(MODULES) -o testes $(DEBUG_FLAGS)
	./testes
#	@G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind -q ./testes 
#	@gcc $(CFLAGS) src/$(TARGET).c  -o $(TARGET)  
#	@./tainted test.oy
#	@rm testes

valgrind-heap:
	@gcc $(CFLAGS) tests/test_all.c $(MODULES) -o testes $(DEBUG_FLAGS)
	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=yes ./testes
	@rm ./testes

valgrind-test:
	@gcc $(CFLAGS) tests/test_all.c $(MODULES) -o testes $(DEBUG_FLAGS)
	G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --num-callers=20 ./testes
	@rm ./testes
