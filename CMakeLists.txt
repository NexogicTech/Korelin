cmake_minimum_required(VERSION 4.1)
project(Korelin C)

set(CMAKE_C_STANDARD 11)

add_executable(Korelin src/korelin.c
        src/korelin.cpp
        src/krilib.c
        src/krilib.h
        src/kapi.c
        src/kapi.h
        src/korelin.h
        src/kvm.c
        src/kvm.h
        src/kgc.c
        src/kgc.h
        src/kparser.c
        src/kparser.h
        src/kstruct.c
        src/kstruct.h
        src/kric.c
        src/kric.h
        src/rungo/rungo.c
        src/rungo/rungo.h
        src/rungo/download.c
        src/rungo/download.h
        src/rungo/package.c
        src/rungo/package.h
        src/kpackage.c
        src/kpackage.h
        src/klexer.c
        src/klexer.h
        src/libs/knet.c
        src/libs/knet.h
        src/libs/kmath.c
        src/libs/kmath.h)
