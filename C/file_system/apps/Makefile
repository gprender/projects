CC := gcc
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror -pedantic-errors

all: test01 test02 test03 test04 test05

test01: test01.c ../io/File.h ../io/File.c ../disk/disk.h ../disk/disk.c
	$(CC) $(CFLAGS) -o test01 test01.c ../io/File.c ../disk/disk.c -lm

test02: test02.c ../io/File.h ../io/File.c ../disk/disk.h ../disk/disk.c
	$(CC) $(CFLAGS) -o test02 test02.c ../io/File.c ../disk/disk.c -lm

test03: test03.c ../io/File.h ../io/File.c ../disk/disk.h ../disk/disk.c
	$(CC) $(CFLAGS) -o test03 test03.c ../io/File.c ../disk/disk.c -lm

test04: test04.c ../io/File.h ../io/File.c ../disk/disk.h ../disk/disk.c
	$(CC) $(CFLAGS) -o test04 test04.c ../io/File.c ../disk/disk.c -lm

test05: test05.c ../io/File.h ../io/File.c ../disk/disk.h ../disk/disk.c
	$(CC) $(CFLAGS) -o test05 test05.c ../io/File.c ../disk/disk.c -lm
