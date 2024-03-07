cmake_minimum_required(VERSION 3.28)
project(au C)

set(CMAKE_C_STANDARD 11)

add_executable(au au.c
        file.h
        file.c
        token.c
        token.h
        util.h
        parse.c
        parse.h
        runtime.h
        runtime.c
        std_inlined.c
        std_inlined.h
)
