/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
	kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
	return op1 + op2;
}

#ifndef FILESYS_STUB
int SysCreate(char *filename, int size) {
	// return value
	// 1: success
	// 0: failed
	return kernel->fileSystem->Create(filename, size);
}

OpenFileId SysOpen(char* filename) {
	if(kernel->fileSystem->Open(filename) == NULL)
		return 0;
	return 1;
}

int SysRead(char* buffer, int size, OpenFileId id) {
	return kernel->fileSystem->getCurrentFile()->Read(buffer, size);
}

int SysWrite(char* buffer, int size, OpenFileId id) {
	return kernel->fileSystem->getCurrentFile()->Write(buffer, size);
}

int SysClose(OpenFileId id) {
	kernel->fileSystem->deleteCurrentFile();
	return 1;
}
#endif

#endif /* ! __USERPROG_KSYSCALL_H__ */
