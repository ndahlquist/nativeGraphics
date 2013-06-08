precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying float depth_MVP;

void main() {
    if(depth_MVP < .5)
        discard;
    gl_FragColor = vec4(v_Normal, depth_MVP); // NX_MV, NY_MV, NZ_MV, Depth_MVP
}