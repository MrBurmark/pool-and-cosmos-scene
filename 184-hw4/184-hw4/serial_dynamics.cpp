
#include <stdio.h>
#include "serial_dynamics.h"
#include "variables.h"
#include <math.h>


const float G = 2.8e-3;
const float k = 3.2e2;

vec3 serial_kfunction(const int i, const vec3 inpos, const vec3 invel) {

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
				
				vec3 u = glm::normalize(inpos - o_pos);

				// ensure dynamics are moving toward each other
				if (dr - glm::length(o_pos + 1e-6f*o_vel - (inpos + 1e-6f*invel)) >= 0.0f) {

					vec3 a = 2.0f*mass*o_mass*u*(invel - o_vel)/(mass+o_mass);
					
					// collision plus gravity's contribution rebounds in opposite usual direction
					kvel += -a*u/mass + -G * o_mass * (o_pos - inpos) / (dr*dr*dr); 
				}
				// plus a spring force
				kvel += (radius + o_radius - dr)*u*k/mass;

			} else { // gravity

				kvel += G * o_mass * (o_pos - inpos) / (dr*dr*dr);
			}
		}
	}
	return kvel;
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

				dynamicobjects[i].vel += 5.0f*hit_strength * dir;

			}
		}
	}
}

void reset_dynamics(void) {

	for (int i = 0; i < numdynamicobjects; ++i) {

		dynamicobjects[i].pos = dynamicobjects[i].initpos;
		dynamicobjects[i].vel = dynamicobjects[i].initvel;

		mat4 tr = Transform::translate(dynamicobjects[i].pos.x, dynamicobjects[i].pos.y, dynamicobjects[i].pos.z);
		dynamicobjects[i].transform = glm::transpose(tr) * dynamicobjects[i].partialtransform;
	}

}


void do_serial_dynamics(const float dt) {

	for (int i = 0; i < numdynamicobjects; ++i) {

		vec3 pos = dynamicobjects[i].pos;
		vec3 vel = dynamicobjects[i].vel;
		
		// calculate new positions using combined gravity and collisions
		
		vec3 inpos = pos;
		vec3 invel = vel;
		vec3 kvel = dt*serial_kfunction(i, inpos, invel);
		vec3 kpos = dt*invel;
		
		vec3 newpos = pos + kpos/6.0f;
		vec3 newvel = vel + kvel/6.0f;
		
		inpos = pos + 0.5f*kpos;
		invel = vel + 0.5f*kvel;
		kvel = dt*serial_kfunction(i, inpos, invel);
		kpos = dt*invel;
		
		newpos += kpos/3.0f;
		newvel += kvel/3.0f;
		
		inpos = pos + 0.5f*kpos;
		invel = vel + 0.5f*kvel;
		kvel = dt*serial_kfunction(i, inpos, invel);
		kpos = dt*invel;
		
		newpos += kpos/3.0f;
		newvel += kvel/3.0f;
		
		inpos = pos + kpos;
		invel = vel + kvel;
		kvel = dt*serial_kfunction(i, inpos, invel);
		kpos = dt*invel;
		
		newpos += kpos/6.0f;
		newvel += kvel/6.0f;

		dynamicobjects[i].kpos = newpos;
		dynamicobjects[i].kvel = newvel;
	}

	for (int i = 0; i < numdynamicobjects; ++i) {
		dynamicobjects[i].pos = dynamicobjects[i].kpos;
		dynamicobjects[i].vel = dynamicobjects[i].kvel;
		mat4 tr = Transform::translate(dynamicobjects[i].pos.x, dynamicobjects[i].pos.y, dynamicobjects[i].pos.z);
		dynamicobjects[i].transform = glm::transpose(tr) * dynamicobjects[i].partialtransform;
	}

}
