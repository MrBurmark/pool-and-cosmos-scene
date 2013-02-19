
#ifndef TABLE_DYNAMICS_H
#define TABLE_DYNAMICS_H

void move_balls(float dt);
void reset_balls(void);
void hit_ball(float dirx, float diry, float dirz, 
			  float eyex, float eyey, float eyez, float hit_strength);

#endif
