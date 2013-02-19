
#include <vector>
#include <GL/glut.h>
#include "transform.h"
#include <omp.h>

#ifdef MAINPROGRAM 
#define EXTERN 
#else 
#define EXTERN extern 
#endif 

EXTERN GLfloat amount; // universal amount used for many movements
EXTERN GLfloat hitamount; // used for the force applied when hitting objects
EXTERN vec3 eye; // The (regularly updated) vector coordinates of the eye 
EXTERN vec3 up;  // The (regularly updated) vector coordinates of the up 
EXTERN vec3 center; // The (regularly updated) vector coordinates of the center 
EXTERN mat4 proj;
EXTERN omp_lock_t hit_ball_lock;
EXTERN omp_lock_t hit_dynamic_lock;

#ifdef MAINPROGRAM 
vec3 eyeinit(-2.5f, 1.8f, 0.8f) ; // Initial eye position, also for resets
vec3 upinit(0.0f, 0.0f, 1.0f) ; // Initial up position, also for resets
vec3 centerinit(0.0f, 0.0f, 0.0f) ; // Center look at point 
float fovy = 75.0f ; // For field of view
float aspect = 1.5f;
int w = 1280, h = 720 ; // width and height 
float zNear = 0.1f, zFar = 99.0f;
bool parallel = false;
bool english = true;
#else 
EXTERN vec3 eyeinit ; 
EXTERN vec3 upinit ; 
EXTERN vec3 centerinit ; 
EXTERN int w, h ; 
EXTERN float fovy ; 
EXTERN float aspect ; 
EXTERN float zNear, zFar;
EXTERN bool parallel;
EXTERN bool english;
#endif 

EXTERN GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
enum shape {cube, sphere, teapot} ;

EXTERN vec4 background;

// Lighting parameter array, similar to that in the fragment shader
const int numLights = 20 ; 
EXTERN GLfloat lightposn [4*numLights] ; // Light Positions
EXTERN GLfloat lightcolor[4*numLights] ; // Light Colors
EXTERN GLfloat lightransf[4*numLights] ; // Lights transformed by modelview
EXTERN GLint dynamiclight[numLights]; // the dynamic object the light is attached to, dynamic lights' lightposn is the offset from their dynamic light's position
EXTERN int numused ;                     // How many lights are used 
EXTERN bool enable_lighting ; 
EXTERN bool enable_texture ; 
EXTERN bool wire_frame ; 
EXTERN bool force_up;
const int numsimkeys = 3;
EXTERN unsigned char down[numsimkeys];

// Materials (read from file) 
// With multiple objects, these are colors for each.
EXTERN GLfloat ambient[4] ; 
EXTERN GLfloat diffuse[4] ; 
EXTERN GLfloat specular[4] ; 
EXTERN GLfloat emission[4] ; 
EXTERN GLfloat shininess ; 

const int maxOBJs = 16 ; 
EXTERN int numOBJs ; 
EXTERN struct OBJ {
	char filename[128] ; 
	char texname[128] ;
	int reference;
	GLfloat ambient[4] ; 
	GLfloat diffuse[4] ; 
	GLfloat specular[4] ;
	GLfloat emission[4] ; 
	GLfloat shininess ;
	mat4 transform ; 
	GLint shadowlights[numLights] ;
	bool tabletop ;
	bool textured ;
	GLuint tex; // end unique
	std::vector <vec3> vertices; // start possibly shared
	std::vector <vec3> normals;
	std::vector <vec3> tex_coords;
	std::vector <vec3> colors;
	std::vector <std::vector<int>> faces;
} OBJs[maxOBJs] ;

EXTERN GLuint OBJvertexbuffer;
EXTERN GLuint OBJcolorbuffer;

// For multiple objects, read from a file.  
const int maxobjects = 64 ; 
EXTERN int numobjects ; 
EXTERN struct object {
  shape type ; 
  GLfloat size ;
  GLfloat ambient[4] ; 
  GLfloat diffuse[4] ; 
  GLfloat specular[4] ;
  GLfloat emission[4] ; 
  GLfloat shininess ;
  mat4 transform ; 
} objects[maxobjects] ;

const int maxdynamicobjects = 64 ; 
EXTERN int numdynamicobjects ; 
EXTERN struct dynamicobject {
  vec3 impulse;
  shape type ; 
  GLfloat size ;
  GLfloat ambient[4] ; 
  GLfloat diffuse[4] ; 
  GLfloat specular[4] ;
  GLfloat emission[4] ; 
  GLfloat shininess ;
  mat4 transform ; // full transformation
  mat4 partialtransform ; // contains no translation data, only scale and rotate
  vec3 initpos;
  vec3 initvel;
  vec3 pos;
  vec3 vel;
  vec3 kpos;
  vec3 kvel;
  GLfloat mass;
  GLfloat radius;
} dynamicobjects[maxdynamicobjects] ;

const int maxtableobjects = 16 ; 
EXTERN int numtableobjects ; 
EXTERN struct tableobject {
  vec3 impulse;
  vec3 rotimpulse;
  shape type ; 
  GLfloat size ;
  GLfloat ambient[4] ; 
  GLfloat diffuse[4] ; 
  GLfloat specular[4] ;
  GLfloat emission[4] ; 
  GLfloat shininess ;
  mat4 transform ; // full transformation
  mat4 rotation;
  mat4 partialtransform ; // contains no translation data, only scale and rotate
  vec3 initpos;
  vec3 initvel;
  vec3 initdrot;
  vec3 precolvel;
  vec3 precoldrot;
  vec3 pos;
  vec3 vel;
  vec3 drot;
  GLfloat rolling;
  GLfloat mass;
  GLfloat radius;
  GLfloat I;
} tableobjects[maxtableobjects] ;

EXTERN GLfloat ballstransf[4*maxtableobjects] ; // balls transformed by modelview

const int maxwalls = 18 ; 
EXTERN int numwalls ; 
EXTERN struct wall {
  vec3 pt1;
  vec3 pt2;
  vec3 perp;
} walls[maxwalls] ;

const int maxholes = 6 ; 
EXTERN int numholes ; 
EXTERN struct hole {
  GLfloat radius;
  vec3 pos;
} holes[maxwalls] ;

// Variables to set for fragment shader 
EXTERN GLuint lightcol ; 
EXTERN GLuint lightpos ; 
EXTERN GLuint numusedcol ; 
EXTERN GLuint enablelighting ; 
EXTERN GLuint ambientcol ; 
EXTERN GLuint diffusecol ; 
EXTERN GLuint specularcol ; 
EXTERN GLuint emissioncol ; 
EXTERN GLuint shininesscol ; 
EXTERN GLuint tabletop ; 
EXTERN GLuint shadowlights ; 
EXTERN GLuint numballs ;
EXTERN GLuint balls ; 
EXTERN GLuint enabletexture ; 
EXTERN GLuint texture ; 
EXTERN GLuint texsampler ; 
EXTERN GLuint sMVP ; 
EXTERN GLuint sMV ; 
EXTERN GLuint sN ; 