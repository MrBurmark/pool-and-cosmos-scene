
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <GL/glew.h>
#include "Transform.h" 

using namespace std ;
#include "variables.h" 
#include "readfile.h"
#include "readOBJ.h"
#include "read_raw.h"

// The function below applies the appropriate transform to a 4-vector, hw2
void matransform(stack<mat4> &transfstack, GLfloat * values) {
  mat4 transform = transfstack.top() ; 
  vec4 valvec = vec4(values[0],values[1],values[2],values[3]) ; 
  vec4 newval = valvec * transform ; 
  for (int i = 0 ; i < 4 ; i++) values[i] = newval[i] ; 
}

// Function to read the input data values
bool readvals(stringstream &s, const int numvals, GLfloat * values) {
  for (int i = 0 ; i < numvals ; i++) {
    s >> values[i] ; 
    if (s.fail()) {
      cout << "Failed reading value " << i << " will skip\n" ; 
      return false ;
    }
  }
  return true ; 
}

void readfile(const char * filename) {
  string str, cmd ; 
  ifstream in ;
  in.open(filename) ; 
  if (in.is_open()) {

    // matrix stack to store transforms.  
    stack <mat4> transfstack ; 
    transfstack.push(mat4(1.0)) ;  // identity

	stack <mat4> partialtransfstack ; 
	partialtransfstack.push(mat4(1.0));  // identity

	numOBJs = 0;
	numobjects = 0;
	numdynamicobjects = 0;
	numtableobjects = 0;
	numwalls = 0;
	numholes = 0;

    getline (in, str) ; 
    while (in) {
		if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[0] != '#') && !(str[str.find_first_not_of(" \t\r\n")] == '/' && str[str.find_first_not_of(" \t\r\n")+1] == '/')) {
        // Ruled out comment and blank lines 

        stringstream s(str) ;
        s >> cmd ; 
        int i ; 
        GLfloat values[25] ; // parameters read in from scene file.  
        bool validinput ; // validity of input 

        // Process the light, add it to database.
        // Lighting Command
        if (cmd == "light") { 
          if (numused == numLights) // No more Lights 
            cerr << "Reached Maximum Number of Lights " << numused << " Will ignore further lights\n" ;
          else {
            validinput = readvals(s, 8, values) ; // Position/color for lts.
            if (validinput) {
				dynamiclight[numused] = -1;
				for (i = 0 ; i < 4 ; i++) {
					lightposn[4*numused + i] = values[i];
					lightcolor[4*numused + i] = values[4 + i];
				}
              ++numused ; 
            }
          }
        }

		else if (cmd == "dynamiclight") { 
          if (numused == numLights) // No more Lights 
            cerr << "Reached Maximum Number of Lights " << numused << " Will ignore further lights\n" ;
          else {
            validinput = readvals(s, 8, values) ; // Position/color for lts.
            if (validinput) { 
				
				dynamiclight[numused] = numdynamicobjects - 1; // attaches to the last object
				for (i = 0 ; i < 4 ; i++) {
					lightposn[4*numused + i] = values[i];
					lightcolor[4*numused + i] = values[4 + i];
				}
				++numused ; 
			}
          }
        }

		else if (cmd == "background") {
			validinput = readvals(s, 4, values) ; // colors 
          if (validinput) {
            background.r = values[0];
			background.g = values[1];
			background.b = values[2];
			background.a = values[3];
		  }
        }

        else if (cmd == "ambient") {
          validinput = readvals(s, 4, values) ; // colors 
          if (validinput) 
            for (i = 0 ; i < 4 ; i++) ambient[i] = values[i] ; 
        }
        else if (cmd == "diffuse") {
          validinput = readvals(s, 4, values) ; 
          if (validinput) 
            for (i = 0 ; i < 4 ; i++) diffuse[i] = values[i] ; 
        }
        else if (cmd == "specular") {
          validinput = readvals(s, 4, values) ; 
          if (validinput) 
            for (i = 0 ; i < 4 ; i++) specular[i] = values[i] ; 
        }
        else if (cmd == "emission") {
          validinput = readvals(s, 4, values) ; 
          if (validinput) 
            for (i = 0 ; i < 4 ; i++) emission[i] = values[i] ; 
        }
        else if (cmd == "shininess") {
          validinput = readvals(s, 1, values) ; 
          if (validinput) shininess = values[0] ; 
        }
        else if (cmd == "size") {
          validinput = readvals(s,2,values) ; 
          if (validinput) { w = (int) values[0] ; h = (int) values[1] ; } 
        }
        else if (cmd == "camera") {
          validinput = readvals(s,10,values) ; // 10 values eye cen up fov
          if (validinput) {
			  eyeinit = vec3(values[0], values[1], values[2]);
			  center = vec3(values[3], values[4], values[5]);
			  upinit = Transform::upvector(vec3(values[6], values[7], values[8]), eyeinit - center);
			  fovy = values[9];
          }
        }

        else if (cmd == "sphere" || cmd == "cube" || cmd == "teapot") {
          if (numobjects == maxobjects) // No more objects 
            cerr << "Reached Maximum Number of Objects " << numobjects << " Will ignore further objects\n" ; 
          else {
            validinput = readvals(s, 1, values) ; 
            if (validinput) {
              object * obj = &(objects[numobjects]) ; 
              obj -> size = values[0] ; 
              for (i = 0 ; i < 4 ; i++) {
                (obj -> ambient)[i] = ambient[i] ; 
                (obj -> diffuse)[i] = diffuse[i] ; 
                (obj -> specular)[i] = specular[i] ; 
                (obj -> emission)[i] = emission[i] ;
              }
              obj -> shininess = shininess ; 
              obj -> transform = transfstack.top() ; 
              if (cmd == "sphere") obj -> type = sphere ; 
              else if (cmd == "cube") obj -> type = cube ; 
              else if (cmd == "teapot") obj -> type = teapot ; 
            }
            ++numobjects ; 
          }
        }
		
        else if (cmd == "dynamicsphere" || cmd == "dynamiccube" || cmd == "dynamicteapot") {
          if (numdynamicobjects == maxdynamicobjects) // No more dynamic objects 
            cerr << "Reached Maximum Number of Dynamic Objects " << numdynamicobjects << " Will ignore further dynamic objects\n" ; 
          else {
            validinput = readvals(s, 15, values) ; 
            if (validinput) {
              dynamicobject * obj = &(dynamicobjects[numdynamicobjects]) ; 
              obj -> size = values[0] ; 
			  obj -> mass = values[1] ;
			  obj -> radius = values[2] ;
			  obj -> initpos = vec3(transfstack.top()[3][0] + values[3] + values[9]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX, 
								transfstack.top()[3][1] + values[4] + values[10]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX, 
								transfstack.top()[3][2] + values[5] + values[11]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX );
			  obj -> pos = obj -> initpos;
			  obj -> initvel = vec3(values[6] + values[12]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX,
								values[7] + values[13]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX,
								values[8] + values[14]*2.0f*(rand()-RAND_MAX/2.0f)/(float)RAND_MAX );
              obj -> vel = obj -> initvel;
			  for (i = 0 ; i < 4 ; i++) {
                (obj -> ambient)[i] = ambient[i] ; 
                (obj -> diffuse)[i] = diffuse[i] ; 
                (obj -> specular)[i] = specular[i] ; 
                (obj -> emission)[i] = emission[i] ;
              }
              obj -> shininess = shininess ; 
              obj -> transform = transfstack.top() ; 
			  obj -> partialtransform = partialtransfstack.top() ;
              if (cmd == "dynamicsphere") obj -> type = sphere ; 
              else if (cmd == "dynamiccube") obj -> type = cube ; 
              else if (cmd == "dynamicteapot") obj -> type = teapot ; 
            }
            ++numdynamicobjects ; 
          }
        }

		else if (cmd == "OBJ") {
          if (numOBJs == maxOBJs) // No more OBJs 
            cerr << "Reached Maximum Number of OBJs " << numOBJs << " Will ignore further OBJs\n" ; 
          else {
			  OBJ * obj = &(OBJs[numOBJs]) ; 
			  s >> obj->filename ; 
			  validinput = readvals(s, 0, values) ; 
            if (validinput) {

				obj->reference = -1;
				for (i = 0 ; i < numOBJs ; i++) {
					if (OBJs[i].filename == obj->filename) {
						obj->reference = i;
						break;
					}
				}
				if (obj->reference == -1) {

					obj->reference = numOBJs;
					
					readOBJfile(obj->filename, obj); 
					
					char norm[128] ; 
					s >> norm ; 
					
					if (norm == "norm") { // assumes convex
						
						for (i = 0; i < obj->faces.size(); ++i) {
							
							vector <int>* face = &obj->faces[i];

							if ((*face)[0] == 5 && face->size() >= 11) {

								vec3 vert1 = obj->vertices[(*face)[2]];
								vec3 vert2 = obj->vertices[(*face)[5]];
								vec3 vert3 = obj->vertices[(*face)[8]];

								vec3 norm1 (0.0f);
								vec3 norm (0.0f);
								
								int n;

								for (unsigned int j = 8;  j < face->size();  j+=3) {

									vert2 = obj->vertices[(*face)[j-3]];
									vert3 = obj->vertices[(*face)[j]];
									
									norm = glm::normalize(glm::cross(vert2 - vert1, vert3 - vert2));
									
									norm1 += norm;

									n = obj->normals.size();
									
									obj->normals.push_back(norm);

									(*face)[j-2] = n;
								}

								(*face)[face->size()-2] = n;

								norm1 = glm::normalize(norm1);

								n = obj->normals.size();

								obj->normals.push_back(norm1);

								(*face)[3] = n;

							} else if ((*face)[0] != 5 && face->size() >= 11) {

								vec3 vert1 = obj->vertices[(*face)[2]];
								vec3 vert2 = obj->vertices[(*face)[5]];
								vec3 vert3 = obj->vertices[(*face)[8]];
								
								vec3 norm = glm::normalize(glm::cross(vert2 - vert1, vert3 - vert2));
								
								int n = obj->normals.size();
								
								obj->normals.push_back(norm);
								
								for (unsigned int j = 3;  j < face->size();  j+=3)
									(*face)[j] = n;
							}
						}
					}
				}
				
				for (i = 0 ; i < 4 ; i++) {
					(obj -> ambient)[i] = ambient[i] ; 
					(obj -> diffuse)[i] = diffuse[i] ; 
					(obj -> specular)[i] = specular[i] ; 
					(obj -> emission)[i] = emission[i] ;
				}
				obj -> shininess = shininess ; 
				obj -> transform = transfstack.top() ; 
				obj -> tabletop = false ;
				obj -> textured = false ;
			} else obj->filename[0] = '\0';

            ++numOBJs ; 
          }
        }

		else if (cmd == "OBJtex") {
          if (numOBJs == maxOBJs) // No more OBJs 
            cerr << "Reached Maximum Number of OBJs " << numOBJs << " Will ignore further OBJs\n" ; 
          else {
			  OBJ * obj = &(OBJs[numOBJs]) ; 
			  s >> obj->filename ; 
			  
			  char texname[128] ; 
			  s >> obj->texname ; 

			  
			  validinput = readvals(s, 2, values) ; 
            if (validinput) {


				// set up textures
				bool found = false;
				for (i = 0 ; i < numOBJs ; i++) {
					if (OBJs[i].texname == obj->texname) {
						obj->tex = OBJs[i].tex;
						found = true;
						break;
					}
				}
				if (!found) {

					glEnable(GL_TEXTURE_2D); 

					obj -> tex = raw_texture_load(obj->texname, (int)values[0], (int)values[1]);

					enable_texture = true;
				}
				obj -> textured = true ;

				// set up geometry
				obj->reference = -1;
				for (i = 0 ; i < numOBJs ; i++) {
					if (OBJs[i].filename == obj->filename) {
						obj->reference = i;
						break;
					}
				}
				if (obj->reference == -1) {

					obj->reference = numOBJs;
					
					readOBJfile(obj->filename, obj); 
				}

				for (i = 0 ; i < 4 ; i++) {
					(obj -> ambient)[i] = ambient[i] ; 
					(obj -> diffuse)[i] = diffuse[i] ; 
					(obj -> specular)[i] = specular[i] ; 
					(obj -> emission)[i] = emission[i] ;
				}
				obj -> shininess = shininess ; 
				obj -> transform = transfstack.top() ; 
				obj -> tabletop = false ;
			} else obj->filename[0] = '\0';

			++numOBJs ; 
          }
        }

		else if (cmd == "OBJtable" || cmd == "OBJtabletop") {
          if (numOBJs == maxOBJs) // No more OBJs 
            cerr << "Reached Maximum Number of OBJs " << numOBJs << " Will ignore further OBJs\n" ; 
          else {
			  OBJ * obj = &(OBJs[numOBJs]) ; 
			  s >> obj->filename ; 
			  
			  validinput = readvals(s, 0, values) ; 
            if (validinput) {

				obj->reference = -1;
				for (i = 0 ; i < numOBJs ; i++) {
					if (OBJs[i].filename == obj->filename) {
						obj->reference = i;
						break;
					}
				}
				if (obj->reference == -1) {

					obj->reference = numOBJs;
					
					readOBJfile(obj->filename, obj); 
					
					// mirror vertices and norms to create 4 images in the 4 quadrants
					int numvert = obj->vertices.size();
					int numnorm = obj->normals.size();
					int numfaces = obj->faces.size();
					// vertices
					for (i = 0; i < numvert; ++i) {

						vec3 vert = obj->vertices[i];
						vert.x = -vert.x; // negative x
						
						obj->vertices.push_back(vert);
					}
					for (i = 0; i < numvert; ++i) {
						
						vec3 vert = obj->vertices[i];
						vert.y = -vert.y; // negative y
						
						obj->vertices.push_back(vert);
					}
					for (i = 0; i < numvert; ++i) {
						
						vec3 vert = obj->vertices[i];
						vert.x = -vert.x;
						vert.y = -vert.y; // negative x and y
						
						obj->vertices.push_back(vert);
					}
					// normals
					for (i = 0; i < numnorm; ++i) {
						
						vec3 norm = obj->normals[i];
						norm.x = -norm.x; // negative x

						obj->normals.push_back(norm);
					}
					for (i = 0; i < numnorm; ++i) {
						
						vec3 norm = obj->normals[i];
						norm.y = -norm.y; // negative y
						
						obj->normals.push_back(norm);
					}
					for (i = 0; i < numnorm; ++i) {
						
						vec3 norm = obj->normals[i];
						norm.x = -norm.x;
						norm.y = -norm.y; // negative x and y
						
						obj->normals.push_back(norm);
					}
					// faces
					for (int l = 1; l < 4; ++l) {
						for (i = 0; i < numfaces; ++i) {
							
							vector <int> facecopy = obj->faces[i];
							vector <int> face = obj->faces[i];
							
							if (l == 1 || l == 2) { // draw in reverse order
								if (face[0] == 5) { // fan
									face[2+0] += l*numvert;
									face[2+1] += l*numnorm;
									for (int j = 5, k = face.size()-3;  j < facecopy.size() && k > 4;  j+=3, k-=3){
										face[k+0] = l*numvert + facecopy[j+0];
										face[k+1] = l*numnorm + facecopy[j+1];
										face[k+2] = facecopy[j+2];
									}
								} else {
									for (int j = 2, k = face.size()-3;  j < facecopy.size() && k > 1;  j+=3, k-=3){
										face[k+0] = l*numvert + facecopy[j+0];
										face[k+1] = l*numnorm + facecopy[j+1];
										face[k+2] = facecopy[j+2];
									}
								}
							} else {
								for (int j = 2; j < face.size(); j+=3){
									face[j+0] += l*numvert;
									face[j+1] += l*numnorm;
								}
							}
							
							obj->faces.push_back(face);
						}
					}
				}
				
				for (i = 0 ; i < 4 ; i++) {
					(obj -> ambient)[i] = ambient[i] ; 
					(obj -> diffuse)[i] = diffuse[i] ; 
					(obj -> specular)[i] = specular[i] ; 
					(obj -> emission)[i] = emission[i] ;
				}
				obj -> shininess = shininess ; 
				obj -> transform = transfstack.top() ; 
				if ( cmd == "OBJtabletop" ) {
					obj -> tabletop = true ;
					validinput = readvals(s, numused, values) ; 
					if (validinput)
						for (i = 0; i < numLights; ++i) {
							if (i < numused && values[i] != 0.0f) (obj -> shadowlights[i] = -1) ;
							else (obj -> shadowlights[i] = 0) ;
						}
				} else obj -> tabletop = false ;
				obj -> textured = false ;
			} else obj->filename[0] = '\0';

            ++numOBJs ; 
          }
        }

		else if (cmd == "tablesphere" || cmd == "tablecube" || cmd == "tableteapot") {
          if (numtableobjects == maxtableobjects) // No more table objects 
            cerr << "Reached Maximum Number of Table Objects " << numtableobjects << " Will ignore further table objects\n" ; 
          else {
            validinput = readvals(s, 11, values) ; 
            if (validinput) {
              tableobject * obj = &(tableobjects[numtableobjects]) ; 
              obj -> size = values[0] ; 
			  obj -> mass = values[1] ;
			  obj -> radius = values[2] ;
			  obj -> I = 2.0f*values[1]*values[2]*values[2]/5.0f;
			  obj -> initpos = vec3(transfstack.top()[3][0] + values[3], transfstack.top()[3][1] + values[4], transfstack.top()[3][2] + values[5]) ;
			  obj -> pos = obj -> initpos;
			  obj -> initvel = vec3(values[6], values[7], 0.0f );
			  obj -> vel = obj -> initvel;
			  obj -> initdrot = vec3(values[8], values[9], values[10]);
			  obj -> drot = obj -> initdrot;
			  obj -> rolling = 3;
              for (i = 0 ; i < 4 ; i++) {
                (obj -> ambient)[i] = ambient[i] ; 
                (obj -> diffuse)[i] = diffuse[i] ; 
                (obj -> specular)[i] = specular[i] ; 
                (obj -> emission)[i] = emission[i] ; 
              }
              obj -> shininess = shininess ; 
              obj -> transform = transfstack.top() ; 
			  obj -> partialtransform = partialtransfstack.top() ;
              if (cmd == "tablesphere") obj -> type = sphere ; 
              else if (cmd == "tablecube") obj -> type = cube ; 
              else if (cmd == "tableteapot") obj -> type = teapot ; 
            }
            ++numtableobjects ; 
          }
        }

		else if (cmd == "wall") {
          if (numwalls == maxwalls) // No more wall objects 
            cerr << "Reached Maximum Number of wall Objects " << numwalls << " Will ignore further wall objects\n" ; 
          else {
            validinput = readvals(s, 4, values) ; 
            if (validinput) {
              wall * obj = &(walls[numwalls]) ; 
			  obj -> pt1 = vec3(values[0], values[1], 0.0f);
			  obj -> pt2 = vec3(values[2], values[3], 0.0f);
			  vec3 u = obj->pt2 - obj->pt1;
			  obj -> perp = glm::normalize(glm::cross(u, vec3(0.0f, 0.0f, 1.0f)));
            }
            ++numwalls ; 
          }
        }

		else if (cmd == "hole") {
          if (numholes == maxholes) // No more hole objects 
            cerr << "Reached Maximum Number of hole Objects " << numwalls << " Will ignore further hole objects\n" ; 
          else {
            validinput = readvals(s, 3, values) ; 
            if (validinput) {
              hole * obj = &(holes[numholes]) ; 
			  obj -> radius = values[0];
			  obj -> pos = vec3(values[1], values[2], 0.0f);
            }
            ++numholes ; 
          }
        }

        else if (cmd == "translate") {
          validinput = readvals(s,3,values) ; 
          if (validinput) {
			mat4 &T = transfstack.top() ; 
			T = T * mat4(1.0, 0.0, 0.0, 0.0, 
						 0.0, 1.0, 0.0, 0.0, 
						 0.0, 0.0, 1.0, 0.0, 
						 values[0], values[1], values[2], 1.0) ; 
          }
        }
        else if (cmd == "scale") {
          validinput = readvals(s,3,values) ; 
          if (validinput) {
			  mat4 &T = transfstack.top() ; 
			  T = T * Transform::scale(values[0], values[1], values[2]);
			  mat4 &P = partialtransfstack.top() ; 
			  P = P * Transform::scale(values[0], values[1], values[2]);
          }
        }
        else if (cmd == "rotate") {
          validinput = readvals(s,4,values) ; 
          if (validinput) {
			mat4 &T = transfstack.top() ; 
			vec3 axis = glm::normalize(vec3(values[0], values[1], values[2]));
			T = T * glm::transpose(mat4(Transform::rotate(values[3], axis)));

			mat4 &P = partialtransfstack.top() ; 
			axis = glm::normalize(vec3(values[0], values[1], values[2]));
			P = P * glm::transpose(mat4(Transform::rotate(values[3], axis)));

          }
        }
        
        else if (cmd == "pushTransform") {
          transfstack.push(transfstack.top()) ; 
		  partialtransfstack.push(partialtransfstack.top()) ; 
		}
		else if (cmd == "popTransform") {
          if (transfstack.size() <= 1) 
            cerr << "Stack has no elements.  Cannot Pop\n" ; 
          else transfstack.pop() ; 
		  if (partialtransfstack.size() <= 1) 
            cerr << "Stack has no elements.  Cannot Pop\n" ; 
          else partialtransfstack.pop() ; 
        }
        
        else {
          cerr << "Unknown Command: " << cmd << " Skipping \n" ; 
        }
      }
      getline (in, str) ; 
    }

  // Set up initial position for eye, up and amount
  // As well as booleans 

    eye = eyeinit ; 
	up = upinit ; 
	amount = 5.0f*pi/180.0f;
	hitamount = 3.0f*amount;
	useGlu = false; // don't use the glu perspective/lookat fns
	enable_lighting = true;
	wire_frame = false;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

  }
  else {
    cerr << "Unable to Open Input Data File " << filename << "\n" ; 
    throw 2 ; 
  }
  
}
