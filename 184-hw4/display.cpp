
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Transform.h"

using namespace std ; 
#include "variables.h"
#include "readfile.h"

void transformvec (const GLfloat input[4], GLfloat output[4], mat4 V) {

	vec4 in (input[0], input[1], input[2], input[3]);

	in = V * in;

	for (int i = 0 ; i < 4 ; i++) {
		output[i] = in[i] ; 
	}
}

void drawOBJ(OBJ* obj) { 

	// use own properties
	if (obj -> tabletop){
		glUniform1i(tabletop, obj -> tabletop);
		glUniform1i(numballs, numtableobjects) ;
		glUniform1iv(shadowlights, numLights, obj -> shadowlights) ;
		glUniform4fv(balls, maxtableobjects, ballstransf) ;
	}

	if (obj->textured) { 

		glUniform1i(texture, true);

		glBindTexture(GL_TEXTURE_2D, obj->tex);

	}

	GLfloat* ambient = obj->ambient;
	GLfloat* diffuse = obj->diffuse;
	GLfloat* specular = obj->specular;
	GLfloat* emission = obj->emission;
	GLfloat shininess = obj->shininess;

	// maybe use another OBJ's geometry, if using own, reference is self-referential

	int reference = obj->reference;

	vector <vec3> vertices = OBJs[reference].vertices;
	vector <vec3> normals = OBJs[reference].normals;
	vector <vec3> tex_coords = OBJs[reference].tex_coords;
	vector <vec3> colors = OBJs[reference].colors;
	vector <vector<int>> faces = OBJs[reference].faces;

	// prepare temporary color arrays if .obj specifies them
	GLfloat tmp_ambient[4] = {ambient[0], ambient[1], ambient[2], ambient[3]};
	GLfloat tmp_diffuse[4] = {diffuse[0], diffuse[1], diffuse[2], diffuse[3]};
	GLfloat tmp_specular[4] = {specular[0], specular[1], specular[2], specular[3]};
	GLfloat tmp_emission[4] = {emission[0], emission[1], emission[2], emission[3]};
	GLfloat tmp_shininess = shininess;

	int num_verts = 0;
	int current_color = -1;

	for (unsigned int i = 0; i < faces.size(); ++i) {

		vector <int> current = faces[i];

		if (current[1] != current_color) {
			if (num_verts != 0) {
				glEnd();
				num_verts = 0;
			}
			current_color = current[1];

			if (current_color == 0) {

				glUniform4fv(ambientcol, 1, ambient) ; 
				glUniform4fv(diffusecol, 1, diffuse) ; 
				glUniform4fv(specularcol, 1, specular) ; 
				glUniform4fv(emissioncol, 1, emission) ; 
				glUniform1f(shininesscol, shininess) ; 

			} else {

				vec3 color = colors[current_color];

				tmp_ambient[0] = color[0]/16.0f, tmp_ambient[1] = color[1]/16.0f, tmp_ambient[2] = color[2]/16.0f, tmp_ambient[3] = 1.0f;
				tmp_diffuse[0] = color[0], tmp_diffuse[1] = color[1], tmp_diffuse[2] = color[2], tmp_diffuse[3] = 1.0f;
				tmp_specular[0] = specular[0]*glm::length(color), tmp_specular[1] = specular[1]*glm::length(color), tmp_specular[2] = specular[2]*glm::length(color), tmp_specular[3] = 1.0f;
				tmp_emission[0] = color[0]/16.0f, tmp_emission[1] = color[1]/16.0f, tmp_emission[2] = color[2]/16.0f, tmp_emission[3] = 1.0f;
				tmp_shininess = shininess*glm::length(color);

				glUniform4fv(ambientcol, 1, tmp_ambient) ;
				glUniform4fv(diffusecol, 1, tmp_diffuse) ; 
				glUniform4fv(specularcol, 1, tmp_specular) ; 
				glUniform4fv(emissioncol, 1, tmp_emission) ; 
				glUniform1f(shininesscol, tmp_shininess) ; 

			}

		}


		if (current[0] == 1) {

			if(num_verts != 1) {
				if (num_verts != 0) glEnd();
				num_verts = 1;
				glBegin(GL_POINTS);
			}
		} 

		else if (current[0] == 2) {

			if(num_verts != 2) {
				if (num_verts != 0) glEnd();
				num_verts = 2;
				glBegin(GL_LINES);
			}
		}

		else if (current[0] == 3) {

			if(num_verts != 3) {
				if (num_verts != 0) glEnd();
				num_verts = 3;
				glBegin(GL_TRIANGLES);
			}
		} 

		else if (current[0] == 4){

			if(num_verts != 4) {
				if (num_verts != 0) glEnd();
				num_verts = 4;
				glBegin(GL_QUADS);
			}
		}

		else if (current[0] == 5){

			if (num_verts != 0) glEnd(); // always start new fan
			num_verts = 5;
			glBegin(GL_TRIANGLE_FAN);
		}

		else if (current[0] == 8){

			if (num_verts != 0) glEnd(); // always start new polygon
			num_verts = 8;
			glBegin(GL_POLYGON);
		}


		for (int j = 2; j < current.size(); j+=3) {
			if (current[j+1]) 
				glNormal3f(normals[current[j+1]].x,
				normals[current[j+1]].y,
				normals[current[j+1]].z);
			if (current[j+2]) 
				glTexCoord3f(tex_coords[current[j+2]].x,
				tex_coords[current[j+2]].y,
				tex_coords[current[j+2]].z);
			glVertex3f(vertices[current[j+0]].x,
				vertices[current[j+0]].y,
				vertices[current[j+0]].z);
		}

	}

	if (num_verts != 0) glEnd();

	if (obj->textured) {
		glUniform1i(texture, false);
		glDisable(GL_TEXTURE_2D);
	}

	if (obj->tabletop) glUniform1i(tabletop, false);

}

void display() {
	glClearColor(background.r, background.g, background.b, background.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




	mat4 Vt = Transform::lookAt(eye,center,up) ; 
	mat4 V = glm::transpose(Vt) ; // accounting for row major


	mat4 P = proj;




	glUniform1i(enabletexture, enable_texture);
	glUniform1i(texture, false);

	glUniform1i(enablelighting, enable_lighting) ;
	if (enable_lighting) {

		GLfloat dynamicposn[4];

		for (int i = 0; i < numused; ++i){
			if (dynamiclight[i] >= 0 && lightposn[4*i+3] != 0.0f) { // only move point light sources
				dynamicposn[0] = lightposn[4*i+0] + (dynamicobjects[dynamiclight[i]].pos.x);
				dynamicposn[1] = lightposn[4*i+1] + (dynamicobjects[dynamiclight[i]].pos.y);
				dynamicposn[2] = lightposn[4*i+2] + (dynamicobjects[dynamiclight[i]].pos.z);
				dynamicposn[3] = lightposn[4*i+3];

				transformvec(dynamicposn, &lightransf[4*i], V);
			} else {
				transformvec(&lightposn[4*i], &lightransf[4*i], V);
			}
		}

		glUniform4fv(lightpos, 4*numLights, lightransf) ;
		glUniform4fv(lightcol, 4*numLights, lightcolor) ;
		glUniform1i(numusedcol, numused) ;
	}

	omp_set_lock(&hit_ball_lock);
	{
		for (int i = 0; i < numtableobjects; ++i){

			tableobject * obj = &(tableobjects[i]) ; 

			mat4 MV = V * obj->transform;
			mat3 N = glm::transpose(glm::inverse(mat3(MV)));
			mat4 MVP = P * MV ;

			glUniformMatrix4fv(sMVP, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(sMV, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix3fv(sN, 1, GL_FALSE, &N[0][0]);

			glUniform4fv(ambientcol, 1, obj->ambient) ; 
			glUniform4fv(diffusecol, 1, obj->diffuse) ; 
			glUniform4fv(specularcol, 1, obj->specular) ; 
			glUniform4fv(emissioncol, 1, obj->emission) ; 
			glUniform1f(shininesscol, obj->shininess) ; 

			if (obj -> type == cube) {
				glutSolidCube(obj->size) ; 
			}
			else if (obj -> type == sphere) {
				const int tessel = 20 ; 
				glutSolidSphere(obj->size, tessel, tessel) ; 
			}
			else if (obj -> type == teapot) {
				glutSolidTeapot(obj->size) ; 
			}

			// set up shadow data for tabletop obj
			vec4 pos = V * vec4(tableobjects[i].pos.x, tableobjects[i].pos.y, tableobjects[i].pos.z, 1.0f);

			ballstransf[4*i+0] = pos.x/pos.w;
			ballstransf[4*i+1] = pos.y/pos.w;
			ballstransf[4*i+2] = pos.z/pos.w;
			ballstransf[4*i+3] = tableobjects[i].radius;
		}
	}
	omp_unset_lock(&hit_ball_lock);

	for (int i = 0 ; i < numOBJs ; i++) {

		OBJ * obj = &(OBJs[i]) ; 

		{
			mat4 MV = V * obj->transform;
			mat3 N = glm::transpose(glm::inverse(mat3(MV)));
			mat4 MVP = P * MV ;

			glUniformMatrix4fv(sMVP, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(sMV, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix3fv(sN, 1, GL_FALSE, &N[0][0]);

		}

		drawOBJ(obj);

	}

	for (int i = 0 ; i < numobjects ; i++) {
		object * obj = &(objects[i]) ; 

		{
			mat4 MV = V * obj->transform;
			mat3 N = glm::transpose(glm::inverse(mat3(MV)));
			mat4 MVP = P * MV ;

			glUniformMatrix4fv(sMVP, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(sMV, 1, GL_FALSE, &MV[0][0]);
			glUniformMatrix3fv(sN, 1, GL_FALSE, &N[0][0]);

			//glLoadMatrixf(&MV[0][0]) ; 

			glUniform4fv(ambientcol, 1, obj->ambient) ; 
			glUniform4fv(diffusecol, 1, obj->diffuse) ; 
			glUniform4fv(specularcol, 1, obj->specular) ; 
			glUniform4fv(emissioncol, 1, obj->emission) ; 
			glUniform1f(shininesscol, obj->shininess) ; 
		}

		if (obj -> type == cube) {
			glutSolidCube(obj->size) ; 
		}
		else if (obj -> type == sphere) {
			const int tessel = 20 ; 
			glutSolidSphere(obj->size, tessel, tessel) ; 
		}
		else if (obj -> type == teapot) {
			glutSolidTeapot(obj->size) ; 
		}

	}

	omp_set_lock(&hit_dynamic_lock);
	{
		for (int i = 0 ; i < numdynamicobjects ; i++) {
			dynamicobject * obj = &(dynamicobjects[i]) ; 
			{
				mat3 N = glm::inverse(mat3(V) * mat3(obj->transform));
				mat4 MV = V * obj->transform;
				mat4 MVP = P * MV ;

				glUniformMatrix4fv(sMVP, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(sMV, 1, GL_FALSE, &MV[0][0]);
				glUniformMatrix3fv(sN, 1, GL_FALSE, &N[0][0]);

				glUniform4fv(ambientcol, 1, obj->ambient) ; 
				glUniform4fv(diffusecol, 1, obj->diffuse) ; 
				glUniform4fv(specularcol, 1, obj->specular) ; 
				glUniform4fv(emissioncol, 1, obj->emission) ; 
				glUniform1f(shininesscol, obj->shininess) ; 
			}

			if (obj -> type == cube) {
				glutSolidCube(obj->size) ; 
			}
			else if (obj -> type == sphere) {
				const int tessel = 20 ; 
				glutSolidSphere(obj->size, tessel, tessel) ; 
			}
			else if (obj -> type == teapot) {
				glutSolidTeapot(obj->size) ; 
			}
		}
	}
	omp_unset_lock(&hit_dynamic_lock);

	/*
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
	glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
	glm::vec3(0,0,0), // and looks at the origin
	glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);  // Changes for each model !
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around


	// Get a handle for our "MVP" uniform.
	// Only at initialisation time.
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	*/
	/*
	mat4 transform = V * Transform::translate(0.0f, 0.0f, 2.0f);

	glLoadMatrixf(&transform[0][0]) ; 


	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, OBJvertexbuffer);
	glVertexAttribPointer(
	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	3,                  // size
	GL_FLOAT,           // type
	GL_FALSE,           // normalized?
	0,                  // stride
	(void*)0            // array buffer offset
	);


	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, OBJcolorbuffer);
	glVertexAttribPointer(
	1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	3,                                // size
	GL_FLOAT,                         // type
	GL_FALSE,                         // normalized?
	0,                                // stride
	(void*)0                          // array buffer offset
	);


	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	*/
	/*
	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	// Draw the triangles !
	glDrawElements(
	GL_TRIANGLES,      // mode
	indices.size(),    // count
	GL_UNSIGNED_INT,   // type
	(void*)0           // element array buffer offset
	);
	*/

	glutSwapBuffers();
}
