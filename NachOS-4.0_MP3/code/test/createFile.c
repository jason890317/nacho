#include "syscall.h"

int main(void)
{
	int success= Create("file2.test");
	if (success != 1) MSG("Failed on creating file");
	MSG("Success on creating file0.test");
	Halt();
}

