##The file is a cpp file

#### File system####
It consists of a
1.superblock (contains number of inodes, number of data blocks, total number of blocks, size of blocks)
2.inode(contains the file size, name of file, first block location)
3. data blocks(block data and location of next block)

###NOTE###
The input written to file, must NOT contain ';' character as i have used that to tell the system that it is the end of file. As user might need to use newline character.

##STRUCTURE##

Inititally, select the numbers
1.create	2.Mount 	3.Exit application

after 1,
write the disk name.

after clicking 2,

1:Create file (enter filename)
2:Open file (enter filename)(mode)
3:Read file (enter fd)
4:Write file (fd)
5:Append (fd)
6:Close (fd)
7:Delete (enter filename)
8:List of files 
9:List of open files
10.Unmount

after 3,
application exits.