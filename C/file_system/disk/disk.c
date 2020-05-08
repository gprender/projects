#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_PATH "../disk/vdisk"

const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;


// Read a specified block from a file into the given buffer.
void read_block(int block_num, unsigned char* buffer) {

	FILE* diskfile = fopen(DISK_PATH, "rb");
	if (diskfile == NULL) {
		printf("Unable to open file: \"%s\"\n", DISK_PATH);
		exit(-1);
	}

	fseek(diskfile, block_num * BLOCK_SIZE, SEEK_SET);
	fread(buffer, BLOCK_SIZE, 1, diskfile);

	fclose(diskfile);
}


// Write the given data into a specified block of a file.
void write_block(int block_num, unsigned char* data) {

	FILE* diskfile = fopen(DISK_PATH, "rb+");
	if (diskfile == NULL) {
		printf("Unable to open file: \"%s\"\n", DISK_PATH);
		exit(-1);
	}

	fseek(diskfile, block_num * BLOCK_SIZE, SEEK_SET);
	fwrite(data, BLOCK_SIZE, 1, diskfile);

	fclose(diskfile);
}


// Create a new, zero-initialized disk.
void wipe_disk() {

	FILE* diskfile = fopen(DISK_PATH, "wb+");
	if (diskfile == NULL) {
		printf("Unable to open file: \"%s\"\n", DISK_PATH);
		exit(-1);
	}

	char* zeros = calloc(BLOCK_SIZE * NUM_BLOCKS, 1);
	fwrite(zeros, BLOCK_SIZE * NUM_BLOCKS, 1, diskfile);

	free(zeros);
	fclose(diskfile);
}