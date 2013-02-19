#ifndef OBJECT_DYNAMICS_H
#define OBJECT_DYNAMICS_H

static bool on_device = false;

extern void do_dynamics(double* dynamic_in_out, const unsigned int numdynamicobjects, const unsigned int numdynamicobjects_align, const double dt);

void freeallcuda(void);

#endif