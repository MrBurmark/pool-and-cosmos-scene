# version 130 

out vec4 color ; 
out vec3 mynormal ; 
out vec4 myvertex ; 

uniform bool texture;

void main() {
	if (texture) {
		gl_TexCoord[0] = gl_MultiTexCoord0 ; 
	}
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ; 
    color = gl_Color ; 
    mynormal = gl_Normal ; 
    myvertex = gl_Vertex ; 

}

