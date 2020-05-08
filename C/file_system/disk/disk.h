const int BLOCK_SIZE;
const int NUM_BLOCKS;

void read_block(int block_num, unsigned char* buffer);

void write_block(int block_num, unsigned char* data);

void wipe_disk();