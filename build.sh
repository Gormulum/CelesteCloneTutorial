#!/bin/bash

libs=-luser32
warnings=-Wno-writable-strings

clang -g src/main.cpp -ocelesteClone.exe $libs $warnings