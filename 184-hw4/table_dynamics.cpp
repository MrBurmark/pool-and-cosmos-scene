
#include "table_dynamics.h"
#include "variables.h"

GLfloat slide_mu = 0.2f;
GLfloat ball_mu = 0.1f;
GLfloat k = 4.32e-3f;
GLfloat g = 9.8f;
GLfloat el = 0.87f;
bool reset_table = false;

void fall_holes(void) {

	for (int j = 0; j < numtableobjects; ++j){

		vec3 pos = tableobjects[j].pos;
		if (pos.z < -100.f) continue;
		pos.z = 0.0f;

		for (int i = 0; i < numholes; ++i){

			vec3 holepos = holes[i].pos; // z defaults to 0.0f

			if (glm::length(pos - holepos) <= holes[i].radius) {
				tableobjects[j].pos = vec3(0.0f, 0.0f, -200.0f);
				tableobjects[j].vel = vec3(0.0f);
				tableobjects[j].drot = vec3(0.0f);
			}
		}
	}
}

void hit_balls(GLfloat dt) {

	for (int i = 0; i < numtableobjects; ++i) {
		tableobjects[i].precolvel = tableobjects[i].vel;
		tableobjects[i].precoldrot = tableobjects[i].drot;
	}

	for (int i = 0; i < numtableobjects; ++i) {

		vec3 pos1 = tableobjects[i].pos;
		if (pos1.z < -100.f) continue;

		vec3 vel1 = tableobjects[i].precolvel;
		vec3 drot1 = tableobjects[i].precoldrot;
		GLfloat radius1 = tableobjects[i].radius;
		GLfloat mass1 = tableobjects[i].mass;
		
		for (int j = 0; j < numtableobjects; ++j) {

			vec3 pos2 = tableobjects[j].pos;
			if (pos2.z < -100.f) continue;

			GLfloat radius2 = tableobjects[j].radius;

			GLfloat dist = glm::length(pos2 - pos1);
			
			// ensure the objects are close enough to hit
			if (i!=j && dist < radius1 + radius2) {

				vec3 u = glm::normalize(pos2 - pos1);

				vec3 vel2 = tableobjects[j].precolvel;
				
				// ensure the objects are moving toward each other
				if (dist - glm::length(pos2 + 1e-6f*vel2 - (pos1 + 1e-6f*vel1)) >= 0.0f) {

					vec3 drot2 = tableobjects[j].precoldrot;
					GLfloat mass2 = tableobjects[j].mass;

					// find their speeds in the collision direction
					float cs1 = glm::dot(u, vel1);
					float cs2 = glm::dot(u, vel2);

					// momentum and energy of the objects in the collision direction
					float P = mass1 * cs1 + mass2 * cs2;
					float E2 = mass1 * cs1*cs1 + mass2 * cs2*cs2;

					// conserve momentum and energy
					float a = mass1 * (1 + mass1/mass2);
					float b = -2.0f * P * mass1/mass2;
					float c = P*P/mass2 - E2;

					float dcs1 = cs1;
					if (b*b - 4.0f*a*c >= 0.0f)
						dcs1 = (-b - sqrtf(b*b - 4.0f*a*c)) / (2.0f*a); // the - solution is the one where they collide

					vec3 impulse = (dcs1 - cs1) * u;

					// friction
					vec3 force = mass1*impulse / dt;

					vec3 forward = glm::cross(vec3(0.0f, 0.0f, 1.0f), u); // normalized as u and up perpendicular and normalized
					// contributions from both balls to the velocity seen by ball1 at the contact point
					vec3 velb = (glm::length(drot1) > 0.0f ? glm::cross(drot1, radius1*u) : vec3(0.0f)) + glm::dot(vel1, forward)*forward - (glm::length(drot2) > 0.0f ? glm::cross(drot2, radius2*-u) : vec3(0.0f)) + glm::dot(vel2, forward)*forward;

					vec3 rotimpulse = dt*ball_mu*glm::cross(force, glm::normalize(velb)) / (tableobjects[i].I);

					tableobjects[i].vel += impulse;

					//tableobjects[i].drot += rotimpulse; // breaking motion, no idea why

				} else {
					
					vec3 spring = (dist - radius1)*k*-u/mass1;

					tableobjects[i].vel += spring;
					
				}
			}
		}
	}
}

void hit_walls(GLfloat dt) {

	for (int j = 0; j < numtableobjects; ++j) {

		if (tableobjects[j].pos.z < -100.f) continue;

		vec3 pos (tableobjects[j].pos.x, tableobjects[j].pos.y, 0.0f);
		vec3 vel = tableobjects[j].vel;
		GLfloat radius = tableobjects[j].radius;

		bool hit_point = false;
		bool hit_wall = false;
		vec3 point_hit (0.0f);

		for (int i = 0; i < numwalls; ++i) {

			vec3 pt1 = walls[i].pt1; // z defaults to 0
			vec3 pt2 = walls[i].pt2;
			vec3 perp = walls[i].perp;
			vec3 w = pt2 - pt1;

			GLfloat d = glm::dot(perp, pt1);

			GLfloat s = d - (glm::dot(perp, pos));

			vec3 intercept = pos + s*perp;

			vec3 tmp = glm::normalize(pos - intercept); // points at ball

			// ensure ball hits wall, and ball is moving towards wall
			if (glm::length(pos - intercept) < radius && glm::dot(tmp, vel) < 0.0f) {
				// ensure ball hits between end points
				if (glm::dot(w, intercept - pt1) > 0.0f && glm::dot(-w, intercept - pt2) > 0.0f) {

					hit_wall = true;

					vec3 wallvel = vel - glm::dot(tmp, vel)*tmp;

					vec3 dvel = -2.0f*glm::dot(tmp, vel)*tmp;

					tableobjects[j].vel += dvel;

					// friction

					GLfloat mass = tableobjects[j].mass;
					vec3 drot = tableobjects[j].drot;

					vec3 force = mass*dvel / dt;

					vec3 rad = -radius*tmp;

					vec3 velw = (glm::length(drot) > 0.0f ? glm::cross(drot, rad) : vec3(0.0f)) + wallvel;

					tableobjects[j].drot += dt*slide_mu*glm::cross(force, glm::normalize(velw))/(tableobjects[j].I);
					break;
				} // detect when a ball hits an end point of a wall without hitting the wall itself
				else if (glm::length(pos - pt1) < radius) {
					hit_point = true;
					point_hit = pt1;
				}
				else if (glm::length(pos - pt2) < radius) {
					hit_point = true;
					point_hit = pt2;
				}
			}
		} // avoid double collisions
		if (hit_point && !hit_wall) {

			vec3 tmp = glm::normalize(pos - point_hit); // points at ball

			vec3 wallvel = vel - glm::dot(tmp, vel)*tmp;

			vec3 dvel = -2.0f*glm::dot(tmp, vel)*tmp;

			tableobjects[j].vel += dvel;

			// friction

			GLfloat mass = tableobjects[j].mass;
			vec3 drot = tableobjects[j].drot;

			vec3 force = mass*dvel / dt;

			vec3 rad = -radius*tmp;

			vec3 velw = (glm::length(drot) > 0.0f ? glm::cross(drot, rad) : vec3(0.0f)) + wallvel;

			tableobjects[j].drot += dt*slide_mu*glm::cross(force, glm::normalize(velw))/(tableobjects[j].I);
		}
	}
}

int q = 0;

void move_balls(float dt) {

	fall_holes();
	hit_balls(dt);
	hit_walls(dt);

	// reduce velocity consistently every second
	float energy_loss = pow(el, dt);

	// friction
	for (int i = 0; i < numtableobjects; ++i) {

		// only update if on table
		if (tableobjects[i].pos.z > -100.f) {

			vec3 velg = (glm::length(tableobjects[i].drot) > 0.0f ? glm::cross(tableobjects[i].drot, vec3(0.0f, 0.0f, -tableobjects[i].radius)) : vec3(0.0f)) + tableobjects[i].vel;

			if (glm::length(velg) != 0.0f) {

				GLfloat fric = slide_mu;

				if (glm::length(velg) < 1.0e-1f) {
					// rolling friction is roughly 200x smaller than sliding
					if (tableobjects[i].rolling > 0.005f) tableobjects[i].rolling *= 0.4167f;

					fric *= tableobjects[i].rolling;

				} else tableobjects[i].rolling = 1.0f;

				tableobjects[i].vel += dt*fric*g*glm::normalize(-velg); 

				tableobjects[i].drot += dt*fric*glm::cross(g*tableobjects[i].mass*vec3(0.0f, 0.0f, 1.0f), glm::normalize(velg))/(tableobjects[i].I);

				// friction doesn't stop the balls properly
				tableobjects[i].vel *= energy_loss;
				tableobjects[i].drot *= energy_loss;
			}
		}
	}

	omp_set_lock(&hit_ball_lock);
	{
		if (reset_table) {

			reset_table = false;
			for (int i = 0; i < numtableobjects; ++i) {

				tableobjects[i].pos = tableobjects[i].initpos;
				tableobjects[i].vel = tableobjects[i].initvel;
				tableobjects[i].drot = tableobjects[i].initdrot;
				tableobjects[i].impulse = vec3(0.0f);
				tableobjects[i].rotimpulse = vec3(0.0f);

				tableobjects[i].rotation = mat4(1.0f);
				mat4 tr = Transform::translate(tableobjects[i].pos.x,
					tableobjects[i].pos.y,
					tableobjects[i].pos.z);
				tableobjects[i].transform = glm::transpose(tr) * tableobjects[i].partialtransform;
			}
		} else {

			for (int i = 0; i < numtableobjects; ++i) {

				if (tableobjects[i].pos.z < -100.f) {

					mat4 tr = Transform::translate(tableobjects[i].pos.x,
						tableobjects[i].pos.y,
						tableobjects[i].pos.z);
					tableobjects[i].transform = glm::transpose(tr) * tableobjects[i].partialtransform;
				} else {

					// add in impulses recieved and zero impulse
					tableobjects[i].vel += tableobjects[i].impulse;
					tableobjects[i].impulse = vec3(0.0f);
					tableobjects[i].drot += tableobjects[i].rotimpulse;
					tableobjects[i].rotimpulse = vec3(0.0f);

					// assert that objects maintain 0 velocity in the z-direction
					tableobjects[i].vel.z = 0.0f;

					tableobjects[i].pos += dt*tableobjects[i].vel;

					// assert that objects maintain they initial z
					tableobjects[i].pos.z = tableobjects[i].initpos.z;

					// can't actually see rotation for spheres
					mat4 rotate (1.0f);
					if (glm::length(tableobjects[i].drot) > 0.0f) {
						rotate = mat4(Transform::rotate(dt * glm::length(tableobjects[i].drot),
							glm::normalize(tableobjects[i].drot)));
					}
					tableobjects[i].rotation *= rotate;
					mat4 tr = Transform::translate(tableobjects[i].pos.x,
						tableobjects[i].pos.y,
						tableobjects[i].pos.z);

					tableobjects[i].transform = glm::transpose(tr) * tableobjects[i].partialtransform * tableobjects[i].rotation;
				}
			}
		}
	}
	omp_unset_lock(&hit_ball_lock);
}

void reset_balls(void) {

	reset_table = true;
}

void hit_ball(float dirx, float diry, float dirz, 
	float eyex, float eyey, float eyez, float hit_strength) {

		vec3 dir = glm::normalize(vec3(dirx, diry, dirz));
		vec3 eye (eyex, eyey, eyez);

		for (int i = 0; i < numtableobjects; ++i) {

			vec3 pos = tableobjects[i].pos;
			if (pos.z < -100.f) continue;

			GLfloat r = tableobjects[i].radius;

			GLfloat a = glm::dot(dir, dir);
			GLfloat b = 2.0f*glm::dot(dir, eye-pos);
			GLfloat c = glm::dot(eye-pos, eye-pos) - r*r;

			if (b*b-4.0f*a*c >= 0.0f) { // hit the ball

				GLfloat t = (-b - sqrtf(b*b-4.0f*a*c))/(2.0f*a);

				if (t > 0.0f) { // ensure ball is in front of person

					vec3 hit = eye + t*dir;

					vec3 hit_dir = glm::normalize(pos - hit);

					// update the object's rotation
					vec3 rotimpulse (0.0f);
					if (english) {
						rotimpulse = hit_strength*glm::cross(dir, hit_dir)/(tableobjects[i].I);
					}

					// balls can't move in the z direction
					hit_dir.z = 0.0f; 
					dir.z = 0.0f;

					vec3 impulse = hit_strength*glm::dot(dir, hit_dir)*hit_dir/tableobjects[i].mass;

					omp_set_lock(&hit_ball_lock);
					{
						tableobjects[i].impulse += impulse;
						tableobjects[i].rotimpulse += rotimpulse;
					}
					omp_unset_lock(&hit_ball_lock);

				}
			}
		}
}