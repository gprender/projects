#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/File.h"

// More on file deletion, and how blocks and inode slots get recycled

int main() {

    init();

    // Making some directories
    make_dir("/usr");
	make_dir("/bin");
	make_dir("/usr/resources");
    make_dir("/bin/stuff");

    // Making a multi-block data file
	unsigned char* data = malloc(1024);
    memset(data, 10, 1024);
    make_datafile("/usr/resources/foo", data, 1024);

    // Deleting a superdirectory
    delete_file("/usr");

    // Making some new files, which re-use freed resources.
    make_dir("/new_usr");
    make_dir("/new_usr/more_resources");
    make_datafile("/new_usr/more_resources/bar", data, 1024);

    printf("Notice how new files recycle the deleted inode slots and storage blocks!\n\n");

    return 1;
}