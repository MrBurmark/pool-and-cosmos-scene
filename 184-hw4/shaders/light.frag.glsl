# version 130 

in vec4 color ;
in vec3 mynormal ; 
in vec4 myvertex ; 
in vec4 mytexcoord ;

const int numLights = 20 ; 
uniform bool enablelighting ; // are we lighting at all (global).
uniform vec4 lightposn[numLights] ; // positions of lights 
uniform vec4 lightcolor[numLights] ; // colors of lights
uniform int numused ;               // number of lights used

const int maxtableobjects = 16;
uniform bool tabletop ;
uniform int shadowlights[numLights] ; // if true shadow with balls
uniform int numballs ;
uniform vec4 balls[maxtableobjects] ; // positions and radii of the balls

uniform bool enabletexture ; 
uniform bool texture ; 
uniform sampler2D texsampler ; 

uniform mat4 MV;
uniform mat3 N;

uniform vec4 ambient ; 
uniform vec4 diffuse ; 
uniform vec4 specular ; 
uniform vec4 emission ; 
uniform float shininess ; 

void main (void) 
{
	vec4 finalcolor = color; 
	vec4 texcolor = vec4(0.0f);

	if (texture && enabletexture) {
		
		texcolor = texture2D(texsampler, mytexcoord.st) ; 
		finalcolor = texcolor; // will reset if lighting
	}

    if (enablelighting) {

		vec4 mypos4 = MV * myvertex;

		vec3 myposition = mypos4.xyz / mypos4.w; // dehomogenize

		vec3 eyedirection = normalize( -myposition); // as eye is at origin

		vec3 normal = normalize(N * mynormal);

		if (texture && enabletexture) {
			
			finalcolor = vec4(0.0f, 0.0f, 0.0f, 1.0f); // reset if there is texturing
		} else {
			
			finalcolor = emission + ambient ; //all component wise except mat*
		}

		for (int i = 0; i < numused; i++) {

			vec3 ldirection;
			if (lightposn[i][3] != 0.0){ // point light
				ldirection = lightposn[i].xyz / lightposn[i].w - myposition;
			} else {
				ldirection = lightposn[i].xyz; // directional
			}
			ldirection = normalize(ldirection);

			bool use = true;

			if (tabletop && shadowlights[i] != 0 && dot(ldirection, normal) > 0.0f) { // shadow with this light
						
					for (int j = 0; j < numballs; ++j) {
							
						vec3 pos = balls[j].xyz;
							
						float r = balls[j].w;
							
						float a = dot(ldirection, ldirection);
						float b = 2.0f*dot(ldirection, pos-lightposn[i].xyz);
						float c = dot(pos-lightposn[i].xyz, pos-lightposn[i].xyz) - r*r;
							
						if (b*b-4.0f*a*c >= 0.0f) use = false; // set to shadowed for this light
					}
			}

			if (use) { // if not in shadow
				
				float Dd = max(dot(ldirection, normal), 0.0f);

				if (Dd > 0.0f) { // avoid points not facing the light

					vec3 halfvec = normalize(ldirection + eyedirection);

					float Sd = pow( max(dot(halfvec, normal), 0.0f), shininess );
				
					if (texture && enabletexture) {

						finalcolor += lightcolor[i] * (texcolor*Dd + specular*Sd);
					} else {
					
						finalcolor += lightcolor[i] * (diffuse*Dd + specular*Sd);
					}
				}
			}
		}
    }
	gl_FragColor = finalcolor ; 
}
