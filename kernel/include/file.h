#ifndef KERNEL_FILE_H
#define KERNEL_FILE_H

struct File {
	uint32_t inode;
};

struct File_Node {
	struct File_Node* next;
	struct File* file;
};

#endif
