// based heavily upon hw2

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shaders.h"
#include "Transform.h"
#include "timing.h"
#include <FreeImage.h>

using namespace std ; 

// Main variables in the program.  
#define MAINPROGRAM 
#include "variables.h" 
//#include "object_dynamics.h"
#include "serial_dynamics.h"
#include "table_dynamics.h"
#include "readfile.h" // prototypes for readfile.cpp  
void display(void) ;  // prototype for display function.  
void free_dynamic(void) ;

void reshape(int width, int height){

	w = width; 
	h = height; 
    mat4 mv ; 

	glMatrixMode(GL_PROJECTION);

        aspect = w / (float) h ;

        //if (useGlu) mv = glm::perspective(fovy,aspect,zNear,zFar) ; 
        //else {
          mv = Transform::perspective(fovy,aspect,zNear,zFar) ; 
          mv = glm::transpose(mv) ; // accounting for row major 
        //}
        glLoadMatrixf(&mv[0][0]) ; 

	glViewport(0, 0, w, h);
}

void saveScreenshot(string fname) {
	int pix = w * h;
	BYTE *pixels = new BYTE[3*pix];	
	glReadBuffer(GL_FRONT);
	glReadPixels(0,0,w,h,GL_BGR,GL_UNSIGNED_BYTE, pixels);

	FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, w, h, w * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
	
	std::cout << "Saving screenshot: " << fname << "\n";

	FreeImage_Save(FIF_PNG, img, fname.c_str(), 0);
	delete pixels;
}


void printHelp() {
  std::cout << "\npress 'h' to print this message again.\n" 
			<< "press 'i' to take sreenshot\n"
			<< "press 'j' to enable or disable texturing\n"
			<< "press 'l' to enable or disable lighting\n"
			<< "press 'p' to enable or disable wireframe\n"
	        << "press '+' or '-' to change the strength used when hitting balls.\n" 
            << "press 'r' to reset the pool balls.\n"
			<< "press 't' to reset the space balls.\n"
            << "left click a ball to hit it forwards\n"
			<< "right click' a ball to hit it backwards\n"
			<< "press 'w' 'a' 's' 'd' 'q' 'e' to move around, 'z' 'c' to tilt\n"
			<< "press 'u' to force up to be \"up\", disables tilting\n"
			<< "press 'space' for turbo speed\n"
			<< "press 'up' 'down' 'left' 'right' or mouse drag to look around\n"
            << "press ESC to quit.\n" ;
}

void addkey(unsigned char key) {

	for (int i = 0; i < numsimkeys; ++i)
		// only allow one instance of key in the list
		if (down[i] == key) return; 

	for (int i = 0; i < numsimkeys; ++i)
		if (down[i] == 0) {
			down[i] = key;
			break;
		}
}

void removekey(unsigned char key) {

	// remove all instances of key in the list
	for (int i = 0; i < numsimkeys; ++i)
		if (down[i] == key) {
			down[i] = 0;
		}
}

int screencap = 1;
void keyboard(unsigned char key, int x, int y) {

	stringstream ss;
	switch(key) {

    case 'a':
		addkey(key);
		break;
	case 'd':
		addkey(key);
		break;
	case 'e':
		addkey(key);
		break;
	case 'q':
		addkey(key);
		break;
	case 'w':
		addkey(key);
		break;
	case 's':
		addkey(key);
		break;
	case 'c':
		addkey(key);
		break;
	case 'z':
		addkey(key);
		break;
	case ' ':
		addkey(key);
		break;

	case 'i':
		ss << screencap++;
		saveScreenshot("screenshot" + ss.str() + ".png");
		break;
	case '+':
		hitamount += pi/180.0f;
		printf("new amount = %.0f\n", hitamount*180.0f/pi);
		break;
	case '-':
		if (hitamount > 0.0f) hitamount -= 1.0f*pi/180.0f;
		printf("new amount = %.0f\n", hitamount*180.0f/pi);
		break;
	case 'h':
		printHelp();
		break;
    case 'r': // reset the balls. 
		reset_balls();
		break ;
	case 't': // reset the spheres. 
		reset_dynamics();
		break ;
	case 'l':
		enable_lighting = !enable_lighting;
		break;
	case 'u':
		force_up = !force_up;
		break;
	case 'j':
		enable_texture = !enable_texture;
		break;
	case 'p':
		if (wire_frame) {
			// disable wireframe
			wire_frame = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		} else {
			wire_frame = true;
			// enable wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
		}
		break;
	case 27:  // Escape to quit
		//freeallcuda();
		//free_dynamic();
		exit(0) ;
		break ;
	}
	glutPostRedisplay();
}

void keyboardup(unsigned char key, int x, int y) {
	removekey(key);
}

void specialKey(int key, int x, int y) {
	switch(key) {
	case 100: //left
          addkey(1);
          break;
	case 101: //up
          addkey(2);
          break;
	case 102: //right
          addkey(3);
          break;
	case 103: //down
          addkey(4);
          break;
	}
}

void specialupKey(int key, int x, int y) {
	switch(key) {
	case 100: //left
		removekey(1);
		break;
	case 101: //up
		removekey(2);
		break;
	case 102: //right
		removekey(3);
		break;
	case 103: //down
		removekey(4);
		break;
	}
}

int oldx = 0, oldy = 0;

void mouse(int button, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		oldx = x ;
		oldy = y ;
	}
	if ((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) && state == GLUT_DOWN) { // hit balls and dynamics
		
		// take into account field of view and aspect ratio/differing width/height
		float sx = aspect*2.0f*tanf(fovy*pi/360.0f)*(x - w*0.5f)/(float)w ; // x is left right 0 - w
		float sy = 2.0f*tanf(fovy*pi/360.0f)*(y - h*0.5f)/(float)h ; // y is top down 0 - h
		
		// set center
		vec3 forward = glm::normalize(center-eye);
		vec3 right = glm::normalize(glm::cross(forward, up));
		vec3 down_vec = glm::normalize(glm::cross(forward, right));

		vec3 dir = glm::normalize(forward + sx*right + sy*down_vec);

		if (button == GLUT_LEFT_BUTTON) {

			hit_ball(dir.x, dir.y, dir.z, eye.x, eye.y, eye.z, hitamount) ;

			hit_dynamics(dir.x, dir.y, dir.z, eye.x, eye.y, eye.z, hitamount);

		} else {

			hit_ball(dir.x, dir.y, dir.z, eye.x, eye.y, eye.z, -hitamount) ;

			hit_dynamics(dir.x, dir.y, dir.z, eye.x, eye.y, eye.z, -hitamount);

		}
	}
	glutPostRedisplay();
}

void mousedrag(int x, int y) {

	float dx = (x - oldx) / (float)w ;
	float dy = (y - oldy) / (float)h ; 

	oldx = x ;
	oldy = y ;

	// set center
	vec3 forward = glm::normalize(center-eye);
	vec3 right = glm::normalize(glm::cross(forward, up));
	vec3 down_vec = glm::normalize(glm::cross(forward, right));

	vec3 axis = glm::normalize(dy*right - dx*down_vec); // perpendicular to desired movement direction, normalized

	Transform::rotatehead(amount*0.2f, axis, eye, up, center);

	glutPostRedisplay();
}

void animation (void) {

	tick();

	do_serial_dynamics (dt() > 0.04f ? 0.04f : dt());

	move_balls(dt() > 0.04f ? 0.04f : dt());

	float speed = amount*0.2f;

	for (int i = 0; i < numsimkeys; ++i)
		if (down[i] == ' ') speed *= 6.0f;

	for (int i = 0; i < numsimkeys; ++i) {
		switch(down[i]) {
		case 1: //left
			Transform::turnleft(speed,  eye,  up, center);
			break;
		case 2: //up
			Transform::tiltup(speed,  eye,  up, center);
			break;
		case 3: //right
			Transform::turnleft(-speed, eye,  up, center);
			break;
		case 4: //down 
			Transform::tiltup(-speed,  eye,  up, center);
			break;
		case 'a':
			Transform::strafeleft(speed, eye,  up, center);
			break;
		case 'd':
			Transform::strafeleft(-speed, eye,  up, center);
			break;
		case 'e':
			Transform::strafeup(speed, eye,  up, center);
			break;
		case 'q':
			Transform::strafeup(-speed, eye,  up, center);
			break;
		case 'w':
			Transform::forward(speed, eye,  up, center);
			break;
		case 's':
			Transform::forward(-speed, eye,  up, center);
			break;
		case 'c':
			Transform::tiltright(-speed, eye,  up, center);
			break;
		case 'z':
			Transform::tiltright(speed, eye,  up, center);
			break;
		}
	}
	// force up be up before redisplay
	if (force_up) up = vec3(0.0f, 0.0f, 1.0f);

	glutPostRedisplay ();
}

void init() {
      // Initialize shaders
      vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl") ;
      fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl") ;
      shaderprogram = initprogram(vertexshader, fragmentshader) ; 

      enablelighting = glGetUniformLocation(shaderprogram,"enablelighting") ;
      lightpos = glGetUniformLocation(shaderprogram,"lightposn") ;       
      lightcol = glGetUniformLocation(shaderprogram,"lightcolor") ;       
      numusedcol = glGetUniformLocation(shaderprogram,"numused") ;       
      ambientcol = glGetUniformLocation(shaderprogram,"ambient") ;       
      diffusecol = glGetUniformLocation(shaderprogram,"diffuse") ;       
      specularcol = glGetUniformLocation(shaderprogram,"specular") ;       
      emissioncol = glGetUniformLocation(shaderprogram,"emission") ;       
      shininesscol = glGetUniformLocation(shaderprogram,"shininess") ; 

	  tabletop = glGetUniformLocation(shaderprogram,"tabletop") ; 
	  shadowlights = glGetUniformLocation(shaderprogram,"shadowlights") ; 
	  numballs = glGetUniformLocation(shaderprogram,"numballs") ; 
	  balls = glGetUniformLocation(shaderprogram,"balls") ; 

	  enabletexture = glGetUniformLocation(shaderprogram,"enabletexture") ; 
	  texture = glGetUniformLocation(shaderprogram,"texture") ; 
	  texsampler = glGetUniformLocation(shaderprogram,"texsampler") ; 

}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		cerr << "No scene file specified\n"; 
		exit(-1); 
	}

  	FreeImage_Initialise();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("HW4: Scene");

	GLenum err = glewInit() ; 
	if (GLEW_OK != err) { 
		cerr << "Error: " << glewGetString(err) << endl; 
	} 

	init();
    readfile(argv[1]) ; 

	glutDisplayFunc(display);

	glutReshapeFunc(reshape);
	glutReshapeWindow(w, h);

	glutIdleFunc(animation);

	glutSpecialFunc(specialKey);
	glutSpecialUpFunc(specialupKey);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse) ;
	glutMotionFunc(mousedrag) ; // causes black screen if both mouse buttons pressed at same time, or click if window not the active window

	printHelp();

	glutMainLoop();

	FreeImage_DeInitialise();
	return 0;
}
