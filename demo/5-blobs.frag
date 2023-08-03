// Source: https://www.shadertoy.com/view/MdsXDM

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 pos = fragCoord - 0.5;
  vec2 cir = ((pos.xy*pos.xy+sin(fragCoord.x*18.0+iTime)/25.0*sin(fragCoord.y*7.0+iTime*1.5)/1.0)+fragCoord.x*sin(iTime)/16.0+fragCoord.y*sin(iTime*1.2)/16.0);
  float circles = (sqrt(abs(cir.x+cir.y*0.5)*25.0)*5.0);
  fragColor = vec4(sin(circles*1.25+2.0),abs(sin(circles*1.0-1.0)-sin(circles)),abs(sin(circles)*1.0),1.0);
}
