
const int MSEC_PER_FRAME = 30;

static double t0 = 0 ; 
static double t1, t2 , ts, ta1, ta2, tb1, tb2; 

extern double timestamp () ; 

extern void initialize() ; 
extern double dt() ; 
extern double dta() ; 
extern double dtb() ; 
extern void tick() ; 
extern void ticka() ; 
extern void tickb() ; 
extern bool specu_tick(double time) ;
extern bool specu_ticka(double time) ;
extern bool specu_tickb(double time) ;