// Source: https://www.shadertoy.com/view/MdsXDM

varying vec2 v_pos;

uniform float u_time;

void main() {
  vec2 pos = v_pos - 0.5;
  vec2 cir = ((pos.xy*pos.xy+sin(v_pos.x*18.0+u_time)/25.0*sin(v_pos.y*7.0+u_time*1.5)/1.0)+v_pos.x*sin(u_time)/16.0+v_pos.y*sin(u_time*1.2)/16.0);
  float circles = (sqrt(abs(cir.x+cir.y*0.5)*25.0)*5.0);
  gl_FragColor = vec4(sin(circles*1.25+2.0),abs(sin(circles*1.0-1.0)-sin(circles)),abs(sin(circles)*1.0),1.0);
}
