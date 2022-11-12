#include "syscall.h"

int main(void)
{
	int i;	
	for(i=0; i<21; i++)
	{
		PrintInt(Open("file1.test"));
	}	

	Halt();
}
