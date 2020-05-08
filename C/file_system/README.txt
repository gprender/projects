#---------------------------------#
#          Introduction           #
#---------------------------------#

This is a simulated Unix-style file system that I implemented from
scratch in C. The structure of the file system's source code is as follows:

	/apps : This is where all the test code lives.
	/disk : The disk driver, and the actual file we use to simulate the disk.
	/io	  : The actual implementation of the file system (the interesting stuff!).


#-----------------------------#
#           USAGE             #
#-----------------------------#

EVERYTHING is made to be run from the /apps directory, which
is where all the tests are located. The Makefile in /apps compiles
everything that the file system needs. 

** IF YOU'RE NOT INSIDE OF /apps, NOTHING IS GOING TO WORK **

Once in /apps, the everything should work as expected. The Makefile
compiles the file system code, as well as all the test programs, which
are named "test01.c" through "test05.c". The resulting executables for
the test files are named similarly: "test01" through "test05".


#-----------------------------#
#          TESTING            #
#-----------------------------#

Each test program tests a different part of the file system:

	test01 : Basic read/write functionality of directories and data files,
				both to 'root' and all subdirectories that are created.
				Reading/writing data files which take up >1 block.
	
	test02 : Basic format of the file system. This was kinda vague in the
				specs, so the test just prints the superblock and FBV.
	
	test03 : Basic deletion of files and directories, and the effect on
				the FBV. Also, prevention of reading deleted files.
	
	test04 : More in-depth on file deletion. Recursive file deletion, as
				well as the recycling of old inodes and data blocks.
	
	test05 : Robustness. Recovering the disk state after different types
				of crashes. Specifically, crashes while writing a file,
				and crashes while deleting a file.

					
#---------------------------------#
#         Disk Structure          #
#---------------------------------#
				

Here's how I set up my disk:

	block 0 : superblock
	
	block 1 : Free-block vector
	
	block 2 : "Safety Block". We use this to recover the disk's state 
				after crashing, by storing a few pieces of information.
	
	block 3 : Free-block vector backup. Not very efficient, but it makes
				recovering after crashes WAY easier.
	
	blocks 4-7 : i-node blocks. This gives us room for (512/32)*4 = 64
					i-nodes, which is more than enough for testing.
	
	block 10 : root directory. We have to start with a root directory,
				so we place it at a set block for simplicity's sake.
	
	blocks 11+ : Free space! This is where we make new directories and
					data files.
					
					
#---------------------------------#
#     Reading/Writing Files       #
#---------------------------------#

Nothing too crazy in my implementation here. Given a path (and possibly
some data), I start at the root (block 10) and traverse the directory
tree using i-nodes and directory blocks until I find the immediate 
parent to the file I want to interact with. From here, it's pretty easy
to either return some data in a buffer, or write a new file altogether
by creating a new i-node and updating the parent directory.

Something I did that I found really useful was making a ton of helper
functions for reading/writing different types of data. For example,
I've got functions to find the earliest free i-node slot, write to
a specific i-node slot, find a free data block, mark a block as "in-use"
in the free-block vector, and so on. I used all the functions a lot,
and they helped maintain simplicity and good abstraction in my code.

As a side-note, I haven't explicitly given a function to modify data
files. I would argue that modifying a data file is functionally 
equivalent to deleting it, and then remaking it with modified data.
In this sense, we already have a way to "modify" data files.


#---------------------------------#
#         File Deletion           #
#---------------------------------#

Deleting files was a little more interesting, and it worked the same
for both directory and data files. Just like for reading/writing a file,
we navigate to the immediate parent first.

From here, we do 3 things to delete the file. First, we delete the 
file's i-node from the i-node blocks. Next, we mark all the file's 
storage blocks as "free" in the FBV. Lastly, we remove the file's entry
from the parent directory's storage block.

Notice that we never actually delete the file's data from its own 
storage blocks. This is SUPER important for robustness, as it lets us
recover files if the file system crashes during file deletion.

As an extra fun thing, I made my file deletion recursive, so that
deleting a directory with stuff in it causes all subfiles to be deleted
as well.


#---------------------------------#
#           Robustness            #
#---------------------------------#

This was the hard part! To make my file system able to recover from
crashes, I used two functions, begin() and commit(), which I call at
the beginning and end of every operation that modifies the disk. What
this does is set a flag that says the file system is in the process of
modifying the disk, and backs up some important information. Then, when
the disk-modifying operation is done, commit() lowers the flag to
indicate that everything is safe again. If a crash occurs before 
commit() is called, we can use sys_recover() to restore the disk to its
state BEFORE the operation began. What's nice about using a flag on 
the disk is that we can call sys_recover() whenever we want, and it 
will only recover the disk if a crash actually happened.

So, how do we do this? I found that we could retain robustness while
modifying a file by backing up 3 things: the file's parent directory's
state, the file's i-node, and the file-system's free-block vector.
We use the safety block (block 2) and the FBV-backup block (block 3)
to do this. Backing up the entire FBV isn't great, and there's probably
a more space-efficient way I could do it, but this method is super
simple to implement and debug.

What sys-recover() does is check if the "in-progress" flag is raised
in the safety block. If it is, we know this means that there was a crash
while writing or deleting, so we immediately restore the disk. This
just involves loading blocks 2 and 3, and writing the backed-up data
back to the disk.

The big reason why this method works is that when deleting files, we
never actually delete data from their storage blocks, so we can restore
the disk's state without having to mess around with up to 10 different
blocks.

