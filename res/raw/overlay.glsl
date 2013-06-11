precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;
varying vec2 v_TexCoordinate;

void main() {

    //gl_FragColor = vec4(texture2D(u_Texture, v_TexCoordinate).rgb, 0.8);
	gl_FragColor = vec4(v_TexCoordinate, 0.0, 0.8);
}
