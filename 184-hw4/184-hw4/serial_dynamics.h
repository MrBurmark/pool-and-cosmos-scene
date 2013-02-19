#ifndef SERIAL_DYNAMICS_H
#define SERIAL_DYNAMICS_H

static bool on_serial_device = false;

void reset_dynamics(void);

void do_serial_dynamics(const float dt);

void hit_dynamics(float dirx, float diry, float dirz, 
				  float eyex, float eyey, float eyez, float hit_strength);

#endif