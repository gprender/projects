/**
 * File.c - Written by Graeme Prendergast, Spring 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../disk/disk.h"

#define NUM_INODES 64
#define INODE_SIZE 32


// Print the indicated block in hexdump-like format; useful for debugging
void print_block(int block_num) {

	unsigned char* buffer = malloc(sizeof(char) * BLOCK_SIZE);
	read_block(block_num, buffer);

	for (int i=0; i<BLOCK_SIZE; i++) {

		if (i % 16 == 0) {
			printf("\nByte %03d:  ", i);
		}

		unsigned char b = buffer[i];
		printf("%02x ", b);
	} printf("\n");
	
	free(buffer);
}


// Read a specific inode into the given buffer
void read_inode(int inode_num, unsigned char* buffer) {

	int pos_in_block = ((inode_num % 16) - 1) * INODE_SIZE;
	int block_num = 4 + (inode_num / 16);

	unsigned char* block_buffer = calloc(BLOCK_SIZE, 1);
	read_block(block_num, block_buffer);

	memcpy(buffer, block_buffer + pos_in_block, sizeof(char)*32);

	free(block_buffer);
}


// Write the provided data as an inode at the given index
void write_inode(int inode_num, unsigned char* data) {

	int pos_in_block = ((inode_num % 16) - 1) * INODE_SIZE;
	int block_num = 4 + (inode_num / 16);

	unsigned char* buffer = calloc(BLOCK_SIZE, 1);
	read_block(block_num, buffer);

	memcpy(buffer + pos_in_block, data, sizeof(char)*32);
	write_block(block_num, buffer);

	free(buffer);
}


// Find the earliest free block on the disk using the free-block vector
int find_free_block() {

	int earliest_free_block = -1;

	unsigned char* fbv = malloc(BLOCK_SIZE);
	read_block(1, fbv);

	// For each byte in the free-block vector
	for (int i=0; i<BLOCK_SIZE; i++) {
		unsigned char b = fbv[i];

		// if that byte is > 0, it must indicate one or more free blocks
		if (b > 0) {

			// For each bit in that byte
			for (int j=0; j<8; j++) {
				int bit = b >> (7-j);

				// if the bit is 1, we've found our earliest free block!
				if (bit > 0) {
					earliest_free_block = (i*8) + j;
					break;
				}
			}
			break;
		}
	}

	free(fbv);
	return earliest_free_block;
}


// Find the earliest free inode slot
int find_free_inode() {

	for(int i=4; i<5; i++) {
		unsigned char* buffer = malloc(BLOCK_SIZE);
		read_block(i, buffer);

		for (int j=0; j<BLOCK_SIZE; j+=32) {
			int inode_filesize = *(int*)(buffer + j);

			if (inode_filesize == 0) { // Found a free inode slot!
				return ((i-4)*8 + j/32) + 1;
			}
		}
	}

	return -1;
}


// Mark a certain block as "in-use" in the free-block vector
void mark_block(int block_num) {

	// printf("Marking block %d for use.\n", block_num);

	unsigned char* fbv = malloc(BLOCK_SIZE);
	read_block(1, fbv);

	int byte_num = (block_num / 8);
	int bit_num = (block_num % 8);

	unsigned char byte = fbv[byte_num];
	unsigned char mask = ~(unsigned char)pow(2, 7-bit_num);
	unsigned char new_byte = (byte & mask);

	memcpy(fbv + byte_num, &new_byte, sizeof(char));
	write_block(1, fbv);

	free(fbv);
}


// Unmark a certain block to indicate it is free
void unmark_block(int block_num) {

	// printf("Unmarking block %d\n", block_num);

	unsigned char* fbv = malloc(BLOCK_SIZE);
	read_block(1, fbv);

	int byte_num = (block_num / 8);
	int bit_num = (block_num % 8);

	unsigned char byte = fbv[byte_num];
	unsigned char mask = (unsigned char)pow(2, 7-bit_num);
	unsigned char new_byte = (byte | mask);

	memcpy(fbv + byte_num, &new_byte, sizeof(char));
	write_block(1, fbv);

	free(fbv);
}


// Split a string by the given delimiter
char** str_split(char* str, const char* delim) {

	// printf("Splitting \'%s\' with delim \'%s\'\n", str, delim);

	char** buffer = (char**)malloc(sizeof(char*) * 16);
	for (int i=0; i<16; i++) {
		buffer[i] = (char*)malloc(sizeof(char) * 128);
	}

	char str_arr[128];
	strcpy(str_arr, str);

	int i = 0;
	char* token;

	token = strtok(str_arr, delim);
	while (token != NULL) {
		memcpy(buffer[i], token, strlen(token)+1);
		// printf("Wrote %s to buffer[%d]\n", token, i);
		i++;

		token = strtok(NULL, delim);
	}
	buffer[i] = 0;

	return buffer;
}


// Write an entry in a parent directory block for a child file
void write_entry_to_parent(int child_inode, char* child_fn, int parent_block) {

	// Find the earliest free entry in the parent block
	unsigned char* buffer = malloc(sizeof(char) * BLOCK_SIZE);
	read_block(parent_block, buffer);

	int entry_num = -1;

	for (int i=0; i<BLOCK_SIZE; i += 32) {
		unsigned char byte = buffer[i];
		if (byte == 0) {
			entry_num = i/32;
			break;
		}
	}

	if (entry_num == -1) {
		printf("The specified directory is already full!\n");
		return;
	}

	// printf("Entry %d is free\n", entry_num);

	// Construct the entry in the parent directory
	unsigned char child_inode_byte = (unsigned char)child_inode;
	unsigned char* entry = calloc(32, sizeof(char));
	memcpy(entry, &child_inode_byte, 1);
	memcpy(entry + 1, child_fn, strlen(child_fn) + 1);

	// Write the entry to the parent block's buffer
	memcpy(buffer + (entry_num * 32), entry, 32);

	// Write the block back onto the disk
	write_block(parent_block, buffer);

	free(entry);
	free(buffer);
}


// Traverse the directory tree to find the data block of the direct parent directory
int find_parent_block(char* path) {

	// Split up the path by forward slashes
	const char* fslash = "/";
	char** split_path = str_split(path, fslash);

	// Figure out how long the path is
	int path_len = 0;
	for ( ; split_path[path_len] != NULL; path_len++);

	int parent_block = 10; // tree traversal always starts at the root (block 10)

	// We only need to traverse if the new directory isn't being made in root
	if (path_len > 1) {

		// Initial traversal setup; this is where things get a little complicated
		int depth = 0;
		int current_inode = 1; // root is always inode 1
		char* current_dir = "";
		char* goal_dir = split_path[path_len - 2];
		unsigned char* inode_buffer = malloc(INODE_SIZE);
		unsigned char* block_buffer = malloc(BLOCK_SIZE);

		// Traverse until we've hit the goal parent directory
		while (strcmp(current_dir, goal_dir) != 0) {
			int found = 0;

			read_block(parent_block, block_buffer);

			// For each entry in the directory block
			int current_entry = 0;
			for ( ; current_entry<16; current_entry++) {

				// Convert the entry's hex filename to a readable string
				char* entry_fn = (char*)&(block_buffer[(current_entry*32)+1]);

				// If it's the same as the one we're looking for, we're done
				if (strcmp(entry_fn, split_path[depth]) == 0) {
					current_dir = entry_fn;
					found = 1;
					break;
				}
			}

			if (!found) {
				printf("The file \'%s\' does not exist!\n", split_path[depth]);
				exit(-1);
			}

			current_inode = block_buffer[current_entry*32];
			read_inode(current_inode, inode_buffer);

			int* inode_flags = (int*)(inode_buffer + 4);
			if (*inode_flags != 0) {
				printf("The file \'%s\' is not a directory!\n", split_path[depth]);
				exit(-1);
			}

			parent_block = inode_buffer[8] + (inode_buffer[9] << 8);
			depth++;
		}

		free(inode_buffer);
		free(block_buffer);
	}

	return parent_block;
}


// Find the inode of the file at the end of the given path
int find_inode_num(char* path) {

	// Split up the path by forward slashes
	const char* fslash = "/";
	char** split_path = str_split(path, fslash);

	// Figure out how long the path is
	int path_len = 0;
	for ( ; split_path[path_len] != NULL; path_len++);

	// Get the block number of the file's parent
	int parent_block = find_parent_block(path);

	// Traverse 1 extra level to get to our data file's inode
	unsigned char* block_buffer = malloc(BLOCK_SIZE);
	read_block(parent_block, block_buffer);

	int found = 0;
	int current_entry = 0;
	for ( ; current_entry<16; current_entry++) {
		char* entry_fn = (char*)(block_buffer + (current_entry*32+1));

		if (strcmp(entry_fn, split_path[path_len-1]) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		printf("The file \'%s\' does not exist!\n", split_path[path_len-1]);
		exit(-1);
	}

	char current_inode = block_buffer[current_entry*32];
	free(block_buffer);

	return (int)current_inode;
}


// If the the file system crashed, recover the previous disk state
void sys_recover() {

	printf("Recovering disk state...\n\n");

	unsigned char* block_buffer = malloc(BLOCK_SIZE);
	read_block(2, block_buffer);

	if (block_buffer[0] == 1) { // There was a crash! Restore the disk!

		// Get all the metadata from the safety block
		short parent_block_num = *(short*)(block_buffer+1);
		char entry_num = *(char*)(block_buffer+3);
		char inode_num = *(char*)(block_buffer+4);

		// restore parent block state (entry)
		unsigned char* parent_buffer = malloc(BLOCK_SIZE);
		unsigned char* entry_buffer = malloc(32);

		read_block(parent_block_num, parent_buffer);
		memcpy(entry_buffer, block_buffer+32, 32);

		// write the entry back into the parent
		memcpy(parent_buffer + (entry_num*32), entry_buffer, 32);
		write_block(parent_block_num, parent_buffer);

		// restore the inode
		memcpy(entry_buffer, block_buffer+64, 32);
		write_inode((int)inode_num, entry_buffer);

		// lower the "working" flag
		char zero = 0;
		memcpy(block_buffer, &zero, 1);
		write_block(2, block_buffer);

		// restore the FBV
		read_block(3, block_buffer);
		write_block(1, block_buffer);

		free(entry_buffer);
		free(parent_buffer);
	}
	
	free(block_buffer);
}


// Commit changes to the disk
// This should be called at the end of each disk-modifying operation
void commit() {

	// Lower the "working" flag
	unsigned char* block_buffer = malloc(BLOCK_SIZE);
	read_block(2, block_buffer);

	char zero = 0;
	memcpy(block_buffer, &zero, 1);

	write_block(2, block_buffer);
}


// Back up the corruptable disk sections when modifying the file @ path
// This should be called at the beginning of each disk-modifying operation
void begin(char* path) {

	char inode_num = (char)find_free_inode();

	unsigned char* safety_buffer = calloc(BLOCK_SIZE, 1);
	char one = 1;
	memcpy(safety_buffer, &one, 1); // "working" flag

	// Find + backup the parent directory block + entry number + entry
	const char* fslash = "/";
	char** split_path = str_split(path, fslash);
	int path_len = 0;
	for ( ; split_path[path_len] != NULL; path_len++);
	short parent_block_num = (short)find_parent_block(path);

	memcpy(safety_buffer+1, &parent_block_num, 2);

	unsigned char* block_buffer = malloc(BLOCK_SIZE);
	read_block(parent_block_num, block_buffer);

	char first_free_entry = -1;
	char entry_num = -1;
	unsigned char* entry_buffer = calloc(32, 1);
	for (int i=0; i<BLOCK_SIZE; i+=32) {
		char* current_entry = (char*)(block_buffer + i);

		if (*current_entry == 0 && first_free_entry == -1) {
			first_free_entry = i/32;
		}

		if (strcmp(split_path[path_len-1], current_entry+1) == 0){
			inode_num = *current_entry;
			entry_num = i/32;
			memcpy(entry_buffer, current_entry, 32);
		}
	}
	if (entry_num == -1) {
		entry_num = first_free_entry;
	}

	memcpy(safety_buffer+3, &entry_num, 1);
	memcpy(safety_buffer+4, &inode_num, 1);
	memcpy(safety_buffer+32, entry_buffer, 32);

	// Back up the FBV to block 3
	read_block(1, block_buffer);
	write_block(3, block_buffer);

	// Find + backup the file's inode
	free(entry_buffer);
	entry_buffer = calloc(32, 1);
	if (inode_num) { // The file has an inode (aka, it exists on disk)
		read_inode(inode_num, entry_buffer);
	}
	memcpy(safety_buffer + 64, entry_buffer, 32);

	// Write all this stuff to the safety block (block 2)
	write_block(2, safety_buffer);


	free(entry_buffer);
	free(block_buffer);
	free(safety_buffer);
	free(split_path);
}


// Make a directory file at the given path
void make_dir(char* path) {

	begin(path);

	// Split up the path by forward slashes
	const char* fslash = "/";
	char** split_path = str_split(path, fslash);

	// Figure out how long the path is
	int path_len = 0;
	for ( ; split_path[path_len] != NULL; path_len++);

	// Next, we need to traverse the tree to find where to make the new directory
	int parent_block = find_parent_block(path);

	// Find some free space to write our new directory to
	int inode_num = find_free_inode();
	int block_num = find_free_block();

	// Write an entry in the parent directory's block
	write_entry_to_parent(inode_num, split_path[path_len-1], parent_block);

	// Construct an inode for the new directory
	unsigned char* buffer = calloc(32, 1);

	unsigned int size = 512;
	memcpy(buffer, &size, sizeof(int));

	unsigned int flags = 0; // indicates this file is a directory
	memcpy(buffer + 4, &flags, sizeof(int));

	// For simplicity, each directory will only use 1 block
	unsigned short short_blocknum = (unsigned short)block_num;
	for (int i=0; i<10; i++) {
		int offset = 8 + (i*2);
		memcpy(buffer + offset, &short_blocknum, sizeof(short));
	}

	// Single/double indirect blocks? idk what these fields mean
	unsigned short zero = 0;
	memcpy(buffer + 28, &zero, sizeof(short));
	memcpy(buffer + 30, &zero, sizeof(short));

	write_inode(inode_num, buffer);
	free(buffer);

	// Veryify that the directory's block on disk is zero-initialized
	unsigned char* zbuffer = calloc(BLOCK_SIZE, 1);
	write_block(block_num, zbuffer);
	free(zbuffer);

	mark_block(block_num); // Mark the directory's block as in-use

	printf("Created a directory at \'%s\':\nParent block %d, inode # %d, storage block %d\n\n",
			path, parent_block, inode_num, block_num);

	// Free the split path buffer
	for (int i=0; i<5; i++) {
		free(split_path[i]);
	} free(split_path);

	commit();
}


/**
 * Make a data file at the given path, provided some data and its size.
 * If you pass an inaccurate data size, you're going to get garbage
 * in the data blocks. So don't do that. Please.
 */
void make_datafile(char* path, unsigned char* data, int data_size) {

	begin(path);

	// Split up the path by forward slashes
	const char* fslash = "/";
	char** split_path = str_split(path, fslash);

	// Figure out how long the path is
	int path_len = 0;
	for ( ; split_path[path_len] != NULL; path_len++);

	// Set up the file's metadata
	int parent_block = find_parent_block(path);
	int inode_num = find_free_inode();
	write_entry_to_parent(inode_num, split_path[path_len-1], parent_block);

	// Figure out which blocks we'll use to store the data (1 or more)
	int nblocks = (data_size / (BLOCK_SIZE+1)) + 1;
	unsigned char* block_nums = calloc(nblocks, 1);
	for (int i=0; i<nblocks; i++) {
		int next_blocknum = find_free_block();
		block_nums[i] = next_blocknum;
		mark_block(next_blocknum);
	}

	// Construct an inode for the new data file
	unsigned char* inode_buffer = calloc(32, 1);

	memcpy(inode_buffer, &data_size, sizeof(int));

	unsigned int flags = 1; // indicates this file is a data file
	memcpy(inode_buffer + 4, &flags, sizeof(int));

	// For simplicity, each directory will only use 1 block
	unsigned short short_blocknum;
	for (int i=0; i<10; i++) {
		if (i < nblocks) {
			short_blocknum = (unsigned short)block_nums[i];
		} else {
			short_blocknum = (unsigned short)block_nums[nblocks-1];
		}

		int offset = 8 + (i*2);
		memcpy(inode_buffer + offset, &short_blocknum, sizeof(short));
	}

	// Single/double indirect blocks? idk what these fields mean
	unsigned short zero = 0;
	memcpy(inode_buffer + 28, &zero, sizeof(short));
	memcpy(inode_buffer + 30, &zero, sizeof(short));

	write_inode(inode_num, inode_buffer);
	free(inode_buffer);

	// Write the actual data to the disk
	unsigned char* block_buffer;
	for (int i=0; i<nblocks; i++) {

		int chunk_size;
		if (i == nblocks-1) {
			if ((data_size % BLOCK_SIZE) == 0) {
				chunk_size = BLOCK_SIZE;
			} else {
				chunk_size = data_size % BLOCK_SIZE;
			}
		} else {
			chunk_size = BLOCK_SIZE;
		}

		block_buffer = calloc(BLOCK_SIZE, 1);
		memcpy(block_buffer, data + i*BLOCK_SIZE, chunk_size);
		write_block(block_nums[i], block_buffer);
		free(block_buffer);
	}

	printf("Created a data file at \'%s\':\nParent block %d, inode # %d, data blocks ",
			path, parent_block, inode_num);
	for(int i=0; i<nblocks; i++) {
		printf("%d ", block_nums[i]);
	} printf("\n\n");

	free(block_nums);
	for (int i=0; i<5; i++) {
		free(split_path[i]);
	} free(split_path);

	commit();
}


// Read the data file at the specified path
// The returned pointer should be freed to avoid memory leaks.
unsigned char* read_file(char* path) {

	printf("Reading the file at \'%s\'\n\n", path);

	int inode_num = find_inode_num(path);

	unsigned char* inode_buffer = malloc(INODE_SIZE);
	read_inode(inode_num, inode_buffer);

	int inode_flags = *(int*)(inode_buffer + 4);
	if (inode_flags != 1) {
		printf("The file \'%s\' is not a data file!\n", path);
		exit(-1);
	}

	// Grab all the file's metadata
	int file_size = *(int*)inode_buffer;
	int nblocks = (file_size/(BLOCK_SIZE+1))+1;
	int* data_blocks = calloc(nblocks, sizeof(int));
	for (int i=0; i<nblocks; i++) {
		data_blocks[i] = (int)inode_buffer[(i*2)+8];
	} 

	unsigned char* data_buffer = calloc(file_size, 1);
	unsigned char* read_buffer;
	for (int i=0; i<nblocks; i++) {
		int block_num = data_blocks[i];
		read_buffer = calloc(BLOCK_SIZE, 1);
		read_block(block_num, read_buffer);

		// Find how many bytes we should read from this block
		int chunk_size;
		if (i == nblocks-1) {
			if ((file_size % BLOCK_SIZE) == 0) {
				chunk_size = BLOCK_SIZE;
			} else {
				chunk_size = file_size % BLOCK_SIZE;
			}
		} else {
			chunk_size = BLOCK_SIZE;
		}

		memcpy(data_buffer + i*BLOCK_SIZE, read_buffer, chunk_size);

		free(read_buffer);
	}

	free(data_blocks);
	free(inode_buffer);

	return data_buffer;
}


// Recursive helper function to delete subfiles, if any exist
void recursive_delete(int inode_num) {

	unsigned char* inode_buffer = malloc(INODE_SIZE);
	read_inode(inode_num, inode_buffer);
	int inode_flags = *(int*)(inode_buffer + 4);

	// If the file is a directory, recursive delete all subfiles
	if (inode_flags == 0) {
		char dir_block = *(char*)(inode_buffer + 8);
		
		unsigned char* block_buffer = malloc(BLOCK_SIZE);
		read_block(dir_block, block_buffer);

		for (int i=0; i<BLOCK_SIZE; i+=32) {
			char inode_byte =  block_buffer[i];
			if(inode_byte > 0) {
				recursive_delete(inode_byte);
			}
		}

		free(block_buffer);
	}

	// For each block used by this file, unmark that block in the FBV
	for (int i=0; i<10; i++) {
		short block_num = *(short*)(inode_buffer + (i*2 + 8));
		unmark_block((int)block_num);
	}

	// Lastly, delete the inode from our inode storage blocks
	unsigned char* blank_inode = calloc(32, 1);
	write_inode(inode_num, blank_inode);

	free(blank_inode);
	free(inode_buffer);
}


// Delete the specified file, as well as any subfiles
void delete_file(char* path) {

	begin(path);

	printf("Deleting \'%s\'\n\n", path);

	int inode_num = find_inode_num(path);
	recursive_delete(inode_num);

	// Remove this entry from the parent directory
	int parent_block = find_parent_block(path);
	unsigned char* block_buffer = malloc(BLOCK_SIZE);
	read_block(parent_block, block_buffer);

	unsigned char* blank_entry = calloc(32, 1);
	for (int i=0; i<BLOCK_SIZE; i+=32) {
		if (block_buffer[i] == inode_num) {
			memcpy(block_buffer + i, blank_entry, 32);
			write_block(parent_block, block_buffer);
			break;
		}
	}

	free(blank_entry);
	free(block_buffer);

	commit();
}


// Simulate a crash while writing a file at path -- for testing purposes
void simulate_write_crash(char* path, unsigned char* data, int data_len) {

	printf("Simulating a crash while writing a file at %s...\n", path);

	make_datafile(path, data, data_len);

	// re-raise the "working" flag that was just lowered by make_datafile()
	unsigned char* buffer = malloc(BLOCK_SIZE);
	read_block(2, buffer);

	char one = 1;
	memcpy(buffer, &one, 1);
	write_block(2, buffer);
}


// Simulate a crash while deleting a file at path -- for testing purposes
void simulate_delete_crash(char* path) {

	printf("Simulating a crash while deleting a file at %s...\n", path);

	delete_file(path);

	// re-raise the "working" flag that was just lowered by delete_file()
	unsigned char* buffer = malloc(BLOCK_SIZE);
	read_block(2, buffer);

	char one = 1;
	memcpy(buffer, &one, 1);
	write_block(2, buffer);
}


// Initialize the root directory -- Blocks 4-7 are our inode blocks
void init_root() {

	// First, allocate the inode
	unsigned char* buffer = calloc(32, 1);

	unsigned int size = 512; // default size of a directory file
	memcpy(buffer, &size, sizeof(int));

	int flags = 0; // indicates root is a directory
	memcpy(buffer + 4, &flags, sizeof(int));

	// The root directory only uses block 10
	unsigned short ten = 10;
	for (int i=0; i<10; i++) {
		int offset = 8 + (i*2);
		memcpy(buffer + offset, &ten, sizeof(short));
	}

	// Single/double indirect blocks? idk what these fields mean
	unsigned short zero = 0;
	memcpy(buffer + 28, &zero, sizeof(short));
	memcpy(buffer + 30, &zero, sizeof(short));

	write_inode(1, buffer);
	free(buffer);

	// We know block 10 is zero-initialized, so we'll just mark it as in-use.
	mark_block(10);
}


// Initialize the free-block vector (block 1)
void init_fbv() {

	unsigned char* buffer = calloc(BLOCK_SIZE, 1);

	unsigned char b = 0; // Blocks 0-7 are unavailable
	memcpy(buffer + sizeof(char)*0, &b, sizeof(char));

	b = 0x3f; // Blocks 8-9 are also unavailable
	memcpy(buffer + sizeof(char)*1, &b, sizeof(char));

	// Every other block is available
	for (int i=2; i<BLOCK_SIZE; i++) {
		b = 0xff;
		memcpy(buffer + sizeof(char)*i, &b, sizeof(char));
	}

	write_block(1, buffer);
	free(buffer);
}


// Initialize the superblock (block 0) with our file system's metadata
void init_superblock() {

	unsigned char* buffer = calloc(BLOCK_SIZE, 1);
	int magic_number = 0xBEEF;
	int blocks = NUM_BLOCKS;
	int inodes = NUM_INODES;

	memcpy(buffer + sizeof(int)*0, &magic_number, sizeof(int));
	memcpy(buffer + sizeof(int)*1, &blocks, sizeof(int));
	memcpy(buffer + sizeof(int)*2, &inodes, sizeof(int));

	write_block(0, buffer);
	free(buffer);
}


// Initialize the file system
void init() {

	wipe_disk();
	init_superblock();
	init_fbv();
	init_root();
}
void InitLLFS() { init(); }

