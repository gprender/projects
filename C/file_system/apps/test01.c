#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/File.h"

// This test shows basic read/write functionality of the file system

int main() {

    init();

    // Make some directories and subdirectories
	make_dir("/usr");
	make_dir("/bin");
	make_dir("/usr/resources");

    // Make a data file
	unsigned char* data = malloc(512);
    memset(data, 10, 512);
    make_datafile("/usr/resources/foo", data, 512);

    // Read the above file
    unsigned char* buffer = read_file("/usr/resources/foo");
    for (int i=0; i<16; i++) {
        printf("%02X ", buffer[i]);
    } printf("...\n\n");

    free(buffer);
    free(data);

    // Let's go bigger! (Reading/writing a multi-block data file)
    unsigned char* big_data = malloc(2048);
    memset(big_data, 10, 2048);
    make_datafile("/bin/largeboi", big_data, 2048);

    unsigned char* big_buffer = read_file("/bin/largeboi");

    // Just reading the start and end of the file
    printf("Start of the file: ");
    for (int i=0; i<8; i++) {
        printf("%02x ", big_buffer[i]);
    } printf("...\n\n");

    printf("End of the file: ");
    for (int i=2040; i<2048; i++) {
        printf("%02x ", big_buffer[i]);
    } printf("END\n\n");

    return 1;
}