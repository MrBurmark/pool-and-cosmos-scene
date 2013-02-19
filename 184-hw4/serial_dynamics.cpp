
#include <stdio.h>
#include "serial_dynamics.h"
#include "variables.h"
#include <math.h>


const float G = 2.8e-3;
const float k = 3.2e2;
bool reset = false;

vec3 serial_kfunction(const int i, const vec3 inpos) {

	vec3 kvel (0.0f);
	GLfloat radius = dynamicobjects[i].radius;

	for (unsigned int j = 0; j < numdynamicobjects; ++j) {
		if (j != i) { // avoid self interactions

			// read in other object's properties
			vec3 o_pos = dynamicobjects[j].pos;
			GLfloat o_mass = dynamicobjects[j].mass;

			GLfloat dr = glm::length(o_pos - inpos);

			vec3 o_vel = dynamicobjects[j].vel;

			kvel += G * o_mass * (o_pos - inpos) / (dr*dr*dr);
		}
	}
	return kvel;
}

void serial_collisions(unsigned int i, vec3 inpos, vec3 invel) {

	vec3 kvel (0.0f);
	GLfloat radius = dynamicobjects[i].radius;
	GLfloat mass = dynamicobjects[i].mass;

	// do object dynamics, each thread may have to do multiple items
	for (unsigned int j = 0; j < numdynamicobjects; ++j) {
		if (j != i) { // avoid self interactions

			// read in other object's properties
			vec3 o_pos = dynamicobjects[j].pos;
			GLfloat o_mass = dynamicobjects[j].mass;
			GLfloat o_radius = dynamicobjects[j].radius;

			GLfloat dr = glm::length(o_pos - inpos);

			// ensure dynamics close enough to collide
			if (dr <= radius + o_radius) {

				vec3 o_vel = dynamicobjects[j].vel;

				vec3 u = glm::normalize(o_pos - inpos);

				// ensure dynamics are moving toward each other
				if (dr - glm::length(o_pos + 1e-6f*o_vel - (inpos + 1e-6f*invel)) >= 0.0f) {

					// find their speeds in the collision direction
					float cs1 = glm::dot(u, invel);
					float cs2 = glm::dot(u, o_vel);

					// momentum and energy of the objects in the collision direction
					float P = mass * cs1 + o_mass * cs2;
					float E2 = mass * cs1*cs1 + o_mass * cs2*cs2;

					// conserve momentum and energy
					float a = mass * (1 + mass/o_mass);
					float b = -2.0f * P * mass/o_mass;
					float c = P*P/o_mass - E2;

					float dcs;
					if (b*b - 4.0f*a*c >= 0.0f)
						dcs = (-b - sqrtf(b*b - 4.0f*a*c)) / (2.0f*a); // the - solution is the one where they collide
					else continue;

					vec3 impulse = (dcs - cs1) * u;

					omp_set_lock(&hit_dynamic_lock);
					{
						dynamicobjects[i].impulse += impulse;
					}
					omp_unset_lock(&hit_dynamic_lock);

				} else {
					// spring force

					vec3 spring = (radius + o_radius - dr)*u*k/mass;

					omp_set_lock(&hit_dynamic_lock);
					{
						dynamicobjects[i].impulse += spring;
					}
					omp_unset_lock(&hit_dynamic_lock);

				}
			}
		}
	}
}

void reset_dynamics(void) {

	reset = true;
}


void do_serial_dynamics(const float dt) {

	for (int i = 0; i < numdynamicobjects; ++i) {

		vec3 pos = dynamicobjects[i].pos;
		vec3 vel = dynamicobjects[i].vel;

		// calculate new positions using combined gravity and collisions, probably uncombine this
		
		vec3 inpos = pos;
		vec3 invel = vel;
		vec3 kvel = dt*serial_kfunction(i, inpos);
		vec3 kpos = dt*invel;
		
		vec3 newpos = pos + kpos/6.0f;
		vec3 newvel = vel + kvel/6.0f;
		
		inpos = pos + 0.5f*kpos;
		invel = vel + 0.5f*kvel;
		kvel = dt*serial_kfunction(i, inpos);
		kpos = dt*invel;
		
		newpos += kpos/3.0f;
		newvel += kvel/3.0f;
		
		inpos = pos + 0.5f*kpos;
		invel = vel + 0.5f*kvel;
		kvel = dt*serial_kfunction(i, inpos);
		kpos = dt*invel;
		
		newpos += kpos/3.0f;
		newvel += kvel/3.0f;
		
		inpos = pos + kpos;
		invel = vel + kvel;
		kvel = dt*serial_kfunction(i, inpos);
		kpos = dt*invel;
		
		newpos += kpos/6.0f;
		newvel += kvel/6.0f;

		dynamicobjects[i].kpos = newpos;
		dynamicobjects[i].kvel = newvel;

		serial_collisions(i, pos, vel);
	}

	omp_set_lock(&hit_dynamic_lock);
	{
		if (reset) {

			reset = false;
			for (int i = 0; i < numdynamicobjects; ++i) {
				
				dynamicobjects[i].pos = dynamicobjects[i].initpos;
				dynamicobjects[i].vel = dynamicobjects[i].initvel;
				dynamicobjects[i].impulse = vec3(0.0f);
				mat4 tr = Transform::translate(dynamicobjects[i].pos.x, dynamicobjects[i].pos.y, dynamicobjects[i].pos.z);
				dynamicobjects[i].transform = glm::transpose(tr) * dynamicobjects[i].partialtransform;
			}
		} else {
			for (int i = 0; i < numdynamicobjects; ++i) {
				dynamicobjects[i].pos = dynamicobjects[i].kpos;
				dynamicobjects[i].vel = dynamicobjects[i].kvel + dynamicobjects[i].impulse;
				dynamicobjects[i].impulse = vec3(0.0f);
				mat4 tr = Transform::translate(dynamicobjects[i].pos.x, dynamicobjects[i].pos.y, dynamicobjects[i].pos.z);
				dynamicobjects[i].transform = glm::transpose(tr) * dynamicobjects[i].partialtransform;
			}
		}
	}
	omp_unset_lock(&hit_dynamic_lock);
}

void hit_dynamics(float dirx, float diry, float dirz, 
				  float eyex, float eyey, float eyez, float hit_strength) {

	vec3 dir (dirx, diry, dirz);
	vec3 eye (eyex, eyey, eyez);

	for (int i = 0; i < numdynamicobjects; ++i) {

		vec3 pos = dynamicobjects[i].pos;

		GLfloat r = dynamicobjects[i].radius;

		GLfloat a = glm::dot(dir, dir);
		GLfloat b = 2.0f*glm::dot(dir, eye-pos);
		GLfloat c = glm::dot(eye-pos, eye-pos) - r*r;

		if (b*b-4.0f*a*c >= 0.0f) { // hit the dynamic

			GLfloat t = (-b - sqrtf(b*b-4.0f*a*c))/(2.0f*a);

			if (t > 0.0f) { // ensure dynamic is in front of person

				vec3 impulse = 5.0f*hit_strength * dir;

				omp_set_lock(&hit_dynamic_lock);
				{
					dynamicobjects[i].impulse += impulse;
				}
				omp_unset_lock(&hit_dynamic_lock);

			}
		}
	}
}
