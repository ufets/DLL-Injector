
INJ=injector
DLL=payload.dll
CC=g++
CFLAGS=-m64 -o
SFLAGS=-static-libgcc -static-libstdc++
LFLAGS=-lSecur32

all: $(INJ) $(DLL)

$(INJ): main.cpp
	$(CC) main.cpp $(SFLAGS) $(CFLAGS) $(INJ) 
$(DLL): payload.cpp
	$(CC) -shared payload.cpp $(CFLAGS) $(DLL) $(LFLAGS)
