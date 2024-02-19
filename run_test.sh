#!/bin/bash

CC=gcc
C_FLAGS='-O0 -Wall -Wextra -pedantic -std=c11'

$CC test/test.c -o test/test && test/test
