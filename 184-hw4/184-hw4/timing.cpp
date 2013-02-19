#include <time.h>
#include "timing.h"

double
timestamp (){
	time_t tv = clock();
	return tv/(double)CLOCKS_PER_SEC;
}

void
initialize(){
	t0 = timestamp();
	t1 = timestamp();
	t2 = timestamp();
}

double
dt(){
	return t2 - t1;
}

void
tick(){
	t1 = t2;
	t2 = timestamp();
}
