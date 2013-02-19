
#include "Transform.h"

namespace Transform {
vec3 upvector(const vec3 &up, const vec3 &z) {

	return glm::normalize(glm::cross(z, glm::cross(up, z)));
}

mat4 scale(const float &sx, const float &sy, const float &sz) {

	return mat4(sx, 0.0f, 0.0f, 0.0f,
				0.0f, sy, 0.0f, 0.0f,
				0.0f, 0.0f, sz, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
}

mat4 translate(const float &tx, const float &ty, const float &tz) {

	return mat4(1.0f, 0.0f, 0.0f, tx,
				0.0f, 1.0f, 0.0f, ty,
				0.0f, 0.0f, 1.0f, tz,
				0.0f, 0.0f, 0.0f, 1.0f);
}

mat3 rotate(const float radians, const vec3& axis) { // assumes axis is normalized

	return cos(radians)*mat3(1.0f)
				+
				(1-cos(radians))*mat3(axis[0]*axis[0], axis[1]*axis[0], axis[2]*axis[0],
									  axis[0]*axis[1], axis[1]*axis[1], axis[2]*axis[1],
									  axis[0]*axis[2], axis[1]*axis[2], axis[2]*axis[2])
				+
				sin(radians)*mat3(0.0f, -axis[2], axis[1],
								  axis[2], 0.0f, -axis[0],
								  -axis[1], axis[0], 0.0f);
}


void rotatehead(float radians, vec3 axis, vec3& eye, vec3& up, vec3& center) {

	mat3 Rot = rotate(radians, axis);
	center = Rot*(center-eye) + eye;
	up = Rot*up;
}

void turnleft(float radians, vec3& eye, vec3& up, vec3& center) {
  
	mat3 Left = rotate(radians, glm::normalize(up));
	center = (center-eye)*Left + eye;
}

void strafeleft(float amount, vec3& eye, vec3& up, vec3& center) {

	vec3 Left = glm::cross(glm::normalize(up), glm::normalize(center-eye));
	eye += amount*Left;
	center += amount*Left;
}

void tiltup(float radians, vec3& eye, vec3& up, vec3& center) {

	mat3 Up = rotate(radians, glm::normalize(glm::cross(up, center-eye)));
	center = Up*(center-eye) + eye;
	up = Up*up;
}

void strafeup(float amount, vec3& eye, vec3& up, vec3& center) {

	vec3 w = glm::normalize(eye-center);
	vec3 u = glm::normalize(glm::cross(up, w));
	vec3 Up = glm::cross(w, u);
	eye += amount*Up;
	center += amount*Up;
}

void forward(float amount, vec3& eye, vec3& up, vec3& center) {
 
	vec3 forward = amount * glm::normalize(center - eye);
	center += forward;
	eye += forward;
}

void tiltright(float radians, vec3& eye, vec3& up, vec3& center) {

	mat3 Up = rotate(radians, glm::normalize(center-eye));
	up = Up*up;
}

mat4 lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) {

	mat4 T = Transform::translate( -eye[0], -eye[1], -eye[2] );
	vec3 w = glm::normalize(eye-center);
	vec3 u = glm::normalize(glm::cross(up, w));
	vec3 v = glm::cross(w, u); // normalized as w, u are normalized
	mat4 R = mat4(u[0], u[1], u[2], 0.0f,
				  v[0], v[1], v[2], 0.0f,
				  w[0], w[1], w[2], 0.0f,
				  0.0f, 0.0f, 0.0f, 1.0f);
  return T*R;
}

mat4 perspective(float fovy, float aspect, float zNear, float zFar){

	float t = zNear*tan(fovy*pi/360);
	float b = -t;
	float r = aspect*t;
	float l = -r;
	return mat4(2.0f*zNear/(r-l), 0.0f, (r+l)/(r-l), 0.0f,
				0.0f, 2.0f*zNear/(t-b), (t+b)/(t-b), 0.0f,
				0.0f, 0.0f, (zFar+zNear)/(zNear - zFar), 2.0f*zFar*zNear/(zNear - zFar),
				0.0f, 0.0f, -1.0f, 0.0f);
}
}
