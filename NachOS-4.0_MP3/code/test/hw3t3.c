#include "syscall.h"

int
main()
{
        int n, i;
        for (n = 1; n < 10; ++n) {
			PrintInt(3);
			for(i=0; i<1000; ++i);
		}
        Exit(3);
}
