CFLAGS=-Wall -I/usr/local/include -lgc
DEBUG_FLAGS=-g3
PROFILE_FLAGS= -fprofile-arcs -ftest-coverage -pg
TARGET=oyster

all:
	gcc $(CFLAGS) $(TARGET).c  -o $(TARGET)  

debug:
	gcc $(CFLAGS) -o $(TARGET) $(TARGET).c $(DEBUG_FLAGS)

profile:
	gcc $(CFLAGS) $(TARGET).c -o $(TARGET) $(PROFILE_FLAGS) 

test:
	@gcc $(CFLAGS) test_all.c -o testes $(DEBUG_FLAGS)
	@ ./testes 
	@rm testes
