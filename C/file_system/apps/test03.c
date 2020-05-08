#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/File.h"

// This test covers how the file system handles file deletion

int main() {

    init();

    // Making some directories
    make_dir("/usr");
	make_dir("/bin");
	make_dir("/usr/resources");
    make_dir("/bin/stuff");

    // Making a data file
	unsigned char* data = malloc(512);
    memset(data, 10, 512);
    make_datafile("/usr/resources/foo", data, 512);

    printf("\nTake note of the free-block vector:");
    print_block(1);
    printf("\n\n");

    // Deleting a directory that has subfiles
    delete_file("/usr/resources");

    printf("\nHere's how the free-block vector looks after that directory deletion:");
    print_block(1);
    printf("\n\n");

    // We can no longer read the subfile, since it has been deleted.
    // The file system intentionally exits after a bad read call. 
    read_file("/usr/resources/foo");

    return 1;
}