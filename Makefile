SHELL = /bin/bash

TARGET = AKPing
OBJS = main.o Ping.o
CPPFLAGS = -std=c++11 -Wall -g
CC = g++
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURR_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
INCLUDEPATH = -I$(CURR_DIR)
INCLUDES = Ping.h
LIBS = 
UNAME := $(shell uname)
FRAMEWORKS = 
ifeq ($(UNAME), Linux)
	FRAMEWORKS += 
	LIBS += 
endif
ifeq ($(UNAME), Darwin)
	FRAMEWORKS += 
	LIBS += 
endif

all : ${TARGET}

${TARGET} : ${OBJS}
	@${CC} ${CPPFLAGS} ${INCLUDEPATH} -o ${TARGET} ${OBJS} ${LIBS} ${FRAMEWORKS}

main.o : main.cpp ${INCLUDES}
	@${CC} ${CPPFLAGS} ${INCLUDEPATH} -c main.cpp

Ping.o : Ping.cpp ${INCLUDES}
	@${CC} ${CPPFLAGS} ${INCLUDEPATH} -c Ping.cpp

${INCLUDES} : 

clean :
	@rm -f ${OBJS} ${TARGET}
