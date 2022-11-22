@echo off

rem build script for windows

set CC=gcc
set FLAGS=-Os -s -Wall -Werror -fstack-protector -std=c99 -pedantic-errors -o

%CC% %FLAGS% -o ..\fart.exe ..\main.c
