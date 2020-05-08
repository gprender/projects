#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/File.h"

// Testing robustness! We have to protect against crashes while reading AND writing

int main() {

    init();

    // This function is exactly the same as a normal data file creation,
    // but it leaves the "working" flag up after finishing.
    simulate_write_crash("/foo", (unsigned char*)"sassafrass", 11);

    sys_recover();

    // trying to read "/foo" at this point causes an intended exit,
    // as the above write resulted in a potentially corrupt file.

    // Notice how this write uses the corrupted inode + data blocks
    make_datafile("/bar", (unsigned char*)"bbbbbbbbbb", 11);

    // Similarly, this function deletes a file, but does not lower
    // the "working" flag after finishing.
    simulate_delete_crash("/bar");

    sys_recover();

    // The file still exists, and we can read from it as normal
    unsigned char* buffer = read_file("/bar");
    for (int i=0; i<11; i++) {
        printf("%c ", buffer[i]);
    } printf("\n\n");

    return 1;
}