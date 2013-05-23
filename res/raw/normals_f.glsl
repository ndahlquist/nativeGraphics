precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying vec3 v_Normal;         	// Interpolated normal for this fragment.

void main() {

    gl_FragColor = vec4(v_Normal, 1.0);

}
