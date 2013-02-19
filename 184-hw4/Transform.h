
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ; 
typedef glm::ivec4 ivec4 ; 
const float pi = 3.14159265f ;

namespace Transform {
vec3 upvector(const vec3 &up, const vec3 &z);
mat3 rotate(const float radians, const vec3& axis) ;
mat4 scale(const float &sx, const float &sy, const float &sz) ; 
mat4 translate(const float &tx, const float &ty, const float &tz);
void rotatehead(float radians, vec3 axis, vec3& eye, vec3& up, vec3& center);
void turnleft(float radians, vec3& eye, vec3& up, vec3& center);
void strafeleft(float amount, vec3& eye, vec3& up, vec3& center);
void tiltup(float radians, vec3& eye, vec3& up, vec3& center);
void strafeup(float amount, vec3& eye, vec3& up, vec3& center);
void forward(float amount, vec3& eye, vec3& up, vec3& center);
void tiltright(float radians, vec3& eye, vec3& up, vec3& center);
mat4 lookAt(const vec3& eye, const vec3 &center, const vec3& up);
mat4 perspective(float fovy, float aspect, float zNear, float zFar);
}

#endif
