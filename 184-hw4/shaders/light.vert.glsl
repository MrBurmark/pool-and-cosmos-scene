# version 130 

out vec4 color ; 
out vec3 mynormal ; 
out vec4 myvertex ; 
out vec4 mytexcoord ;

uniform bool texture;
uniform mat4 MVP;

//layout(location = 0) in vec3 vertexPosition;
//layout(location = 1) in vec3 vertexColor;

void main() {
	if (texture) {
		mytexcoord = gl_MultiTexCoord0 ;
	}
    gl_Position = MVP * gl_Vertex ; 
    color = gl_Color ; 
    mynormal = gl_Normal; 
    myvertex = gl_Vertex;

}
	/*
	
	
	// Output data ; will be interpolated for each fragment.
	out vec3 fragmentColor;
	
	void main(){
		
    // The color of each vertex will be interpolated
    // to produce the color of each fragment
    fragmentColor = vertexColor;
	}

	// Interpolated values from the vertex shaders
	in vec3 fragmentColor;
	
	
	// Output color = color specified in the vertex shader,
	// interpolated between all 3 surrounding vertices
color = fragmentColor;



	
in vec3 vertexPosition_modelspace;
uniform mat4 MVP;
 
void main(){
 
    // Output position of the vertex, in clip space : MVP * position
    vec4 v = vec4(vertexPosition_modelspace,1); // Transform an homogeneous 4D vector, remember ?
    gl_Position = MVP * v;
}

*/