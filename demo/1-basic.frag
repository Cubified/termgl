#version 110

varying vec2 v_pos;

void main() {
  gl_FragColor = vec4(v_pos, 0.5, 1.0);
}
