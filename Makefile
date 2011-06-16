CFLAGS=-Wall
DEBUG_FLAGS=-g3
PROFILE_FLAGS= -fprofile-arcs -ftest-coverage -pg
TARGET=oyster
all:
	gcc $(CFLAGS) -o $(TARGET) $(TARGET).c 

debug:
	gcc $(CFLAGS) -o $(TARGET) $(TARGET).c $(DEBUG_FLAGS)

profile:
	gcc $(CFLAGS) -o $(TARGET) $(TARGET).c $(PROFILE_FLAGS) 

test:
	@gcc $(CFLAGS) -o testes test_all.c $(DEBUG_FLAGS)
	@ ./testes 
#	@rm testes
