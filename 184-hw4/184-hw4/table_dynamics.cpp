
#include "table_dynamics.h"
#include "variables.h"

GLfloat slide_mu = 0.2f;
GLfloat ball_mu = 0.1f;
GLfloat g = 9.8f;
GLfloat el = 0.87f;

void fall_holes(void) {

	for (int i = 0; i < numholes; ++i){

		vec3 holepos = holes[i].pos; // z defaults to 0.0f

		for (int j = 0; j < numtableobjects; ++j){

			vec3 pos = tableobjects[j].pos;
			pos.z = 0.0f;

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
		vec3 vel1 = tableobjects[i].precolvel;
		vec3 drot1 = tableobjects[i].precoldrot;
		GLfloat radius1 = tableobjects[i].radius;
		GLfloat mass1 = tableobjects[i].mass;
		GLfloat I = tableobjects[i].I;

		for (int j = 0; j < numtableobjects; ++j) {

			vec3 pos2 = tableobjects[j].pos;
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
					
					vec3 a = -2.0f*mass1*mass2*u*(vel1 - vel2)/(mass1+mass2);
					
					tableobjects[i].vel += a*u/mass1;
					
					// friction
					
					vec3 forward = glm::cross(vec3(0.0f, 0.0f, 1.0f), u); // normalized as u and up perpendicular and normalized
					// contributions from both balls to the velocity seen by ball1 at the contact point
					vec3 velb = (glm::length(drot1) > 0.0f ? glm::cross(drot1, radius1*u) : vec3(0.0f)) + glm::dot(vel1, forward)*forward - (glm::length(drot2) > 0.0f ? glm::cross(drot2, radius2*-u) : vec3(0.0f)) + glm::dot(vel2, -forward)*-forward;
					
					tableobjects[i].drot += dt*ball_mu*glm::cross(g*mass1*vec3(0.0f, 0.0f, 1.0f), glm::normalize(velb))/(tableobjects[i].I);
				}
			}
		}
	}
}

void hit_walls(GLfloat dt) {


	for (int j = 0; j < numtableobjects; ++j) {

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

					vec3 force = -mass*2.0f*glm::dot(tmp, vel)*tmp / dt;

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

			vec3 force = -mass*2.0f*glm::dot(tmp, vel)*tmp / dt;

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

	for (int i = 0; i < numtableobjects; ++i) {

		// assert that objects maintain 0 velocity in the z-direction
		tableobjects[i].vel.z = 0.0f;

		tableobjects[i].pos += dt*tableobjects[i].vel;

		if (tableobjects[i].pos.z > -100.0f) {
			// assert that objects maintain they initial z, unless pocketed
			tableobjects[i].pos.z = tableobjects[i].initpos.z;
		}

		// can't actually see rotation for spheres
		mat4 rotate (1.0f);
		if (glm::length(dt*tableobjects[i].drot) > 0.0f) {
			rotate = mat4(Transform::rotate(glm::length(dt*tableobjects[i].drot),
											glm::normalize(tableobjects[i].drot)));
		}
		mat4 tr = Transform::translate(tableobjects[i].pos.x,
									   tableobjects[i].pos.y,
									   tableobjects[i].pos.z);

		tableobjects[i].transform = glm::transpose(tr) * rotate * tableobjects[i].partialtransform;


		// friction, but currently objects don't stop the balls properly
		tableobjects[i].vel *= energy_loss;
		tableobjects[i].drot *= energy_loss;

		bool drot_loss = false;
		bool vel_loss = false;
		/*
		if (glm::length(tableobjects[i].drot) < 8.0e1f && glm::length(tableobjects[i].drot) != 0.0f) {
			if (glm::length(tableobjects[i].drot) < 4.0e1f) {
				if (glm::length(tableobjects[i].drot) < 1.0e1f) {
					if (glm::length(tableobjects[i].drot) < 5.0e0f) {
						if (glm::length(tableobjects[i].drot) < 1.0e-1f) {
							tableobjects[i].drot *= 0.0f;
						} else tableobjects[i].drot *= 0.05f;
					} else tableobjects[i].drot *= 0.15f;
				} else tableobjects[i].drot *= 0.35f;
			} else 	tableobjects[i].drot *= 0.6f;
			drot_loss = true;
		}
		*/
		if (glm::length(tableobjects[i].vel) < 1.0e-2f && glm::length(tableobjects[i].vel) != 0.0f) {
			if (glm::length(tableobjects[i].vel) < 5.0e-3f) {
				if (glm::length(tableobjects[i].vel) < 1.0e-3f) {
					if (glm::length(tableobjects[i].vel) < 1.0e-4f) {
						tableobjects[i].vel *= 0.0f;
					} else tableobjects[i].vel *= 0.25f;
				} else tableobjects[i].vel *= 0.45f;
			} else tableobjects[i].vel *= 0.65f;
			vel_loss = true;
		}

		vec3 vel = tableobjects[i].vel;
		vec3 drot = tableobjects[i].drot;
		GLfloat r = tableobjects[i].radius;
		GLfloat m = tableobjects[i].mass;

		vec3 velg = (glm::length(drot) > 0.0f ? glm::cross(drot, vec3(0.0f, 0.0f, -r)) : vec3(0.0f)) + vel;

		if (glm::length(velg) != 0.0f) {

			GLfloat fric = slide_mu;
			if (glm::length(velg) < 1.0e-1f) {
				// rolling friction is roughly 200x smaller than sliding
				if (tableobjects[i].rolling < 128) tableobjects[i].rolling *= 2;

				fric /= tableobjects[i].rolling;// detect rolling if close to rolling for 3 time steps

			} else tableobjects[i].rolling = 2;

			if (!vel_loss)
				tableobjects[i].vel += dt*fric*g*glm::normalize(-velg); 

			if (!drot_loss)
				tableobjects[i].drot += dt*fric*glm::cross(g*m*vec3(0.0f, 0.0f, 1.0f), glm::normalize(velg))/(tableobjects[i].I);
		}
	}
}

void reset_balls(void) {

	for (int i = 0; i < numtableobjects; ++i) {

		tableobjects[i].pos = tableobjects[i].initpos;
		tableobjects[i].vel = tableobjects[i].initvel;
		tableobjects[i].drot = tableobjects[i].initdrot;

		mat4 tr = Transform::translate(tableobjects[i].pos.x,
									   tableobjects[i].pos.y,
									   tableobjects[i].pos.z);
		tableobjects[i].transform = glm::transpose(tr) * tableobjects[i].partialtransform;
	}
}

void hit_ball(float dirx, float diry, float dirz, 
			  float eyex, float eyey, float eyez, float hit_strength) {

	vec3 dir = glm::normalize(vec3(dirx, diry, dirz));
	vec3 eye (eyex, eyey, eyez);

	for (int i = 0; i < numtableobjects; ++i) {

		vec3 pos = tableobjects[i].pos;

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
				tableobjects[i].drot += hit_strength*glm::cross(dir, hit_dir)/(tableobjects[i].I);

				// balls can't move in the z direction
				hit_dir.z = 0.0f; 
				dir.z = 0.0f;

				tableobjects[i].vel += hit_strength*glm::dot(dir, hit_dir)*hit_dir/tableobjects[i].mass;

			}
		}
	}
}