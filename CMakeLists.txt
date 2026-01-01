cmake_minimum_required(VERSION 4.1)
project(Korelin C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(Korelin
        src/korelin.c
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
        src/krip/rungo.c
        src/krip/rungo.h
        src/krip/download.c
        src/krip/download.h
        src/krip/package.c
        src/krip/package.h
        src/kpackage.c
        src/kpackage.h
        src/klexer.c
        src/klexer.h
        src/libs/knet.c
        src/libs/knet.h
        src/libs/kmath.c
        src/libs/kmath.h
        src/libs/stdlib.c
        src/libs/stdlib.h
        src/libs/kmap.c
        src/libs/kmap.h
        src/ast.c
        src/ast.h
        src/kevaluator.c
        src/kevaluator.h
)
