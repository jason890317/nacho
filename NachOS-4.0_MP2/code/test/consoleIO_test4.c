#include "syscall.h"

int a[10000];

int main() {
	int i;
	for(i=0; i<3; ++i)
			PrintInt(i);
	return 0;
}
