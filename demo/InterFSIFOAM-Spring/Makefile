#!/bin/bash

CC	= mpic++
#CC      = icx
CFLAGS	= -std=c++14 -O3 -I/home/omar/WORK/OpenFOAM/OpenFOAM-v2206/ThirdParty-v2206/sources/MUI

SCR = $(wildcard *.cpp)
EXE = $(SCR:.cpp=)

default: $(EXE)

% : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXE)
