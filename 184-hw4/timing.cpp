#include <time.h>
#include "timing.h"
#include <omp.h>

double
timestamp (){
	return omp_get_wtime();
}

void
initialize(){
	t0 = timestamp();
	t1 = t0;
	t2 = t0;
	ta1 = t0;
	ta2 = t0;
	tb1 = t0;
	tb2 = t0;
	ts = t0;
}

double
dt(){
	return t2 - t1;
}

double
dta(){
	return ta2 - ta1;
}

double
dtb(){
	return tb2 - tb1;
}

void
tick(){
	t1 = t2;
	t2 = timestamp();
}

void
ticka(){
	ta1 = ta2;
	ta2 = timestamp();
}

void
tickb(){
	tb1 = tb2;
	tb2 = timestamp();
}

bool specu_tick(double time) {
	if (timestamp() - t1 > time) return true;
	return false;
}

bool specu_ticka(double time) {
	if (timestamp() - ta1 > time) return true;
	return false;
}

bool specu_tickb(double time) {
	if (timestamp() - tb1 > time) return true;
	return false;
}
