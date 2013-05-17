
uniform mat4 u_MVPMatrix;		// A constant representing the combined model/view/projection matrix.      		       
uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.
		  			
attribute vec4 a_Position;		// Per-vertex position information we will pass in.   							
attribute vec3 a_Normal;		// Per-vertex normal information we will pass in.      
attribute vec2 a_TexCoordinate; // Per-vertex texture coordinate information we will pass in. 		
		  
varying vec3 v_Position;		// This will be passed into the fragment shader.  		          		
varying vec3 v_Normal;			// This will be passed into the fragment shader.          		
varying vec3 v_Normal_eye;	    // This will be passed into the fragment shader.
varying vec2 v_TexCoordinate;   // This will be passed into the fragment shader.

uniform vec3 u_LightPos;
varying vec3 v_LightPos;
		  
// The entry point for our vertex shader.  
void main() {
	// Pass through the texture and normal coordinates.
	v_TexCoordinate = a_TexCoordinate;
	v_Normal = a_Normal;
	v_Normal_eye = vec3(u_MVMatrix * vec4(a_Normal, 1.0));

	// gl_Position is a special variable used to store the final position.
	// Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
	gl_Position = u_MVPMatrix * a_Position;
	
	// Transform the vertex into eye space.
	v_Position = (vec3(u_MVPMatrix * a_Position) + 1.0 ) / 2.0;
	
	
	vec3 light = vec3(u_MVMatrix * vec4(u_LightPos, 1.0));
	v_LightPos = vec3(light.x / 10.0, light.y / 10.0, (light.z+150.0) / 100.0);
}
