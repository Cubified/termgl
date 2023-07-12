#version 110

uniform float u_time;

varying vec2 v_pos;

void main() {
  gl_FragColor = vec4(sin(u_time), cos(u_time), 0.0, 1.0);
}
