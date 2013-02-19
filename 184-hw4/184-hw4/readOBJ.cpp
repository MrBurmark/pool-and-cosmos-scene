 
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
//#include <GL/glut.h>

using namespace std ;
#include "variables.h" 


// Function to read the input data values and set up a face
bool readintvals(stringstream &s, vector <int> &intvalues) {
  string tmp;
  size_t pos;
  for (int i = 0; !s.eof(); i++) {

	  s >> tmp;
	  
	  intvalues.push_back(atoi (tmp.c_str()));

	  pos = tmp.find("/");
	  if (pos == -1){
		  intvalues.push_back(0);
		  intvalues.push_back(0);
		  continue;
	  }
	  
	  tmp = tmp.substr(pos+1);
	  intvalues.push_back(atoi (tmp.c_str()));

	  pos = tmp.find("/");

	  if (pos == -1){
		  intvalues.push_back(0);
		  continue;
	  }
	  
	  tmp = tmp.substr(pos+1);
	  intvalues.push_back(atoi (tmp.c_str()));
	  
	  if (s.fail()) {
		  cout << "Failed reading value " << i << " will skip\n" ; 
		  return false ;
	  }
  }
  return true ; 
}


// Function to read the input data values
// Use is optional, but should be very helpful in parsing.  
bool readfloatvals(stringstream &s, const int numvals, GLfloat * values, float dflt) {
  for (int i = 0 ; i < numvals ; i++) {
    if(!s.eof()){
      s >> values[i] ; // will fail if ends in spaces or non numeral characters, eof will not be set
      if (s.fail()) {
        cout << "Failed reading value " << i << " will skip\n" ; 
        return false ;
      }
    } else values[i] = dflt;
  }
  return true ; 
}

void readOBJfile(const char * filename, OBJ* obj) {
  string str, cmd ; 
  ifstream in ;
  in.open(filename) ; 
  if (in.is_open()) {
    // I need to implement a matrix stack to store transforms.  
    // This is done using standard STL Templates 

    obj->vertices.push_back(vec3(0.0f)); // OBJ indexing starts at 1
    obj->tex_coords.push_back(vec3(0.0f));
    obj->normals.push_back(vec3(0.0f));
	obj->colors.push_back(vec3(0.0f));

    getline (in, str) ; 
    while (in) {
      if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[str.find_first_not_of(" \t\r\n")] != '#') && !(str[str.find_first_not_of(" \t\r\n")] == '/' && str[str.find_first_not_of(" \t\r\n")+1] == '/')) {
        // Ruled out comment and blank lines 

        stringstream s(str) ;
        s >> cmd ; 
        GLfloat floatvalues[4] ; // vector coordinates
        bool validinput ; // validity of input 

        // Process the vertex, add it to database.
        if (cmd == "v") { 
          validinput = readfloatvals(s, 3, floatvalues, 1.0f) ; // Position for vertex.
          if (validinput) {
              obj->vertices.push_back(vec3(floatvalues[0], floatvalues[1], floatvalues[2]));
          }
        } 
		else if (cmd == "vt") {
			validinput = readfloatvals(s, 3, floatvalues, 0.0f) ; // texture position for vertex.
			if (validinput) {
				obj->tex_coords.push_back(vec3(floatvalues[0], floatvalues[1], floatvalues[2]));
			}
		}
		else if (cmd == "vn") {
			validinput = readfloatvals(s, 3, floatvalues, 0.0f) ; // Normal for vertex.
			if (validinput) {
				obj->normals.push_back(glm::normalize(vec3(floatvalues[0], floatvalues[1], floatvalues[2])));
			}
		}
		else if (cmd == "color") {
			validinput = readfloatvals(s, 3, floatvalues, 0.0f) ; // color for face.
			if (validinput) {
				obj->colors.push_back(vec3(floatvalues[0], floatvalues[1], floatvalues[2]));
			}
		}
		else if (cmd == "f") {
			vector <int> intvalues;
			intvalues.push_back(0);
			intvalues.push_back(obj->colors.size()-1);
			validinput = readintvals(s, intvalues) ; // Position for vertex.
			if (validinput) {
				if(intvalues.size() == 5) intvalues[0] = 1; // point
				else if(intvalues.size() == 8) intvalues[0] = 2; // line
				else if(intvalues.size() == 11) intvalues[0] = 3; // triangle
				else if(intvalues.size() == 14) intvalues[0] = 4; // quad
				else if(intvalues.size() > 14) intvalues[0] = 8; // polygon
				obj->faces.push_back(intvalues);
			}
		}
		else if (cmd == "fan") {
			vector <int> intvalues;
			intvalues.push_back(0);
			intvalues.push_back(obj->colors.size()-1);
			validinput = readintvals(s, intvalues) ; // Position for vertex.
			if (validinput) {
				intvalues[0] = 5; // triangle fan
				obj->faces.push_back(intvalues);
			}
		}
		else {
			cerr << "Unknown Command: " << cmd << " Skipping \n" ; 
		}
	  }
	  getline (in, str) ; 
	}
	
  }
  else {
	  cerr << "Unable to Open Input Data File " << filename << "\n" ; 
	  throw 2 ; 
  }
}
