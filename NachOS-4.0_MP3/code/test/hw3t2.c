#include "syscall.h"

int
main()
{
	int n, i, j;
	for (n = 1; n < 5; ++n) {
		PrintInt(2);
		PrintInt(3);
		for(i=0; i<100; ++i);
	}
	Exit(2);
}
