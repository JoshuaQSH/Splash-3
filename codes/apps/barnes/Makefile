TARGET = BARNES
OBJS = code.o code_io.o load.o grav.o getparam.o util.o

include ../../Makefile.config

stdinc.h: code.h defs.h util.h vectmath.h load.h code_io.h grav.h getparam.h stdinc.h.in
code.c: code.c.in
code_io.c: code_io.c.in
getparam.c: getparam.c.in
grav.c: grav.c.in
load.c: load.c.in
util.c: util.c.in
code.o: code.c.in stdinc.h
code_io.o: code_io.c.in stdinc.h
getparam.o: getparam.c.in stdinc.h
grav.o: grav.c.in stdinc.h
load.o: load.c.in stdinc.h
util.o: util.c.in stdinc.h

