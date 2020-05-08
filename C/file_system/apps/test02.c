#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../io/File.h"

// The whole disk creation/mounting thing is kinda vague, so this test
// will just show how the file system formats itself on initialization.

int main() {

    init();

    printf("\nPrinting superblock...");
    print_block(0);
    printf("\n");

    printf("\nPrinting the free-block vector...");
    print_block(1);
    printf("\n");

    return 1;
}