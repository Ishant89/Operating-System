
#include <stdio.h>
#include <string.h>
#include "traceback.h"

	void bar(int x, int y)
{
	  int z;
	    z = x + y;
	      (void) z;
	        traceback(stdout);
}

void foo() {
	  char buf[4];
	    strncat(buf,"111111288888999991000012222222229888888898888890111111111111111111111111111111111\n",40);
	      bar (5,17);
}

int main (int argc, char **argv)
{
	  foo();
	    return 0;
}
