#version 110
// Adapted from: https://www.shadertoy.com/view/DtyXDR
//
// CC0 1.0 Universal https://creativecommons.org/publicdomain/zero/1.0/
// To the extent possible under law, Blackle Mori has waived all copyright and related or neighboring rights to this work.

varying vec2 v_pos;
uniform float u_time;

vec3 erot(vec3 p, vec3 ax, float ro) {
  return mix(ax*dot(p,ax),p,cos(ro)) + cross(ax,p)*sin(ro);
}

float fact(vec3 p, float s) {
  return dot(asin(sin(p)*s),vec3(1))/sqrt(3.);
}

int id = 0;
float scene(vec3 p) {
  float a1 = fact(erot(p,vec3(1,0,0),2.),.9);
  float a2 = fact(erot(p,vec3(0,1,0),1.5),.9);
  float a3 = fact(erot(p,vec3(0,0,1),2.),.9);
  float a4 = fact(erot(p,normalize(vec3(0,1,1)),.59),.999);
  float s1 = (a1+a2+a3)/2. - 1. + length(p.yz)*.1;
  s1 = min(s1,length(p.yz)-1.);
  float s2 = a4-1.5;
  if (s1 < s2) id = 0;
  else id = 1;
  return id == 0 ? s2 : s1;
}

vec3 norm(vec3 p) {
  mat3 k = mat3(p,p,p) - mat3(0.01);
  return normalize(scene(p) - vec3(scene(k[0]),scene(k[1]),scene(k[2])));
}

vec3 randomdir(float stage) {
  vec3 dotdir = vec3(1);
  if (mod(stage, 5.0)==0.0) dotdir.x *= -1.;
  if (mod(stage, 5.0)==1.0) dotdir.y *= -1.;
  if (mod(stage, 5.0)==2.0) dotdir.z *= -1.;
  if (mod(stage, 5.0)==3.0) dotdir.x *= 0.;
  if (mod(stage, 5.0)==0.0) dotdir.xyz = dotdir.yzx;
  
  return dotdir;
}

void main() {
  vec4 fragColor;
  vec2 uv = v_pos - 0.5;
 
  float t = u_time;
  t += length(uv)*2.5*pow(sin(t*.4)*.5+.5,8.);
  
  float stage0 = t + 0.5;
  float swipedir = (mod(stage0/2.0, 2.0) == 0.0 ? -1.0 : 1.0) * (mod(stage0, 2.0) == 0.0 ? uv.x : uv.y);
  
  float stage = t+swipedir*.25;
  float sdiff = t - stage;
  float stage2 = t*3.5;

  vec3 cam = normalize(vec3(.6+sin(stage*3.1415 + 1.5*smoothstep(-.5,.5,cos(sdiff*3.)))*.3,uv));
  vec3 init = vec3(cos(t*.3)/.3*.5,0,0);
  
  float rdir = (mod(stage, 3.0) == 0.0 ? -1.0 : 1.0);
  if (mod(stage, 5.0) > 1.0)rdir*=0.;
  float zrot = t*.2*rdir + float(stage);
  float yrot = sin(t*.0555*rdir)*.3;
  float xrot = smoothstep(-.65,.65,cos(sdiff*3.))*rdir;
  cam = erot(cam,vec3(1,0,0),xrot);
  cam = erot(cam,vec3(0,1,0),yrot);
  cam = erot(cam,vec3(0,0,1),zrot);
  
  vec3 p = init;
  bool hit = false;
  for ( int i = 0; i < 150 && !hit; i++) {
    if (distance(p,init) > 400.) break;
    float d = scene(p);
    if (d*d < 0.00001) hit = true;
    p += cam*d;
  }
  int lid = id;
  float fog = smoothstep(0., (mod(stage, 7.0)-mod(stage, 5.0) < 0.0)?20.:100., distance(p,init));
  vec3 n = norm(p);
  vec3 r = reflect(cam,n);
  float ao = 1.0;
  ao *= smoothstep(-0.5,.5,scene(p+n*.5));
  ao *= smoothstep(-1.,1.,scene(p+n*1.));
  ao *= smoothstep(-2.,2.,scene(p+n*2.));
  float fres = 1.-abs(dot(cam,n))*.98;
  float rfact = length(sin(r*3.5)*.5+.5)/sqrt(3.);
  float dfact = length(sin(n*2.5)*.4+.6)/sqrt(3.);
  rfact = rfact*.2 + pow(rfact,8.)*6.;
  
  
  float stripes = asin(sin(dot(randomdir(stage),p)*3.+float(t)));
  if (mod(stage, 4.0) == 0.0) stripes *= asin(sin(dot(randomdir(stage+7.0),p)*3.+float(t)));
  vec3 dcolor = vec3(0.85,0.4,0.05);
  if (mod(stage, 3.0) == 0.0) dcolor = vec3(0.85,0.1,0.05);
  if (mod(stage, 3.0) == 1.0) dcolor = vec3(0.85,0.8,0.05);
  if (0 == lid) dcolor = dcolor.zxy;
  else dcolor*=smoothstep(-0.05,0.05,stripes);
  
  
  vec3 col = (vec3(rfact*fres) + dcolor*dfact)*ao;
  
  if (!hit) fog = 1.0;
  fragColor.xyz = mix(col, vec3(1.1), fog);
  fragColor.xyz *= 1. - dot(uv,uv)*.8;
  fragColor.xyz = smoothstep(0.,1.,sqrt(fragColor.xyz));
  if (mod(stage, 4.0) == 1.0) fragColor.xyz = 1.-fragColor.xyz;
  if (mod(stage, 4.0) == 3.0) fragColor.xyz = 1.-fragColor.xxx;
  //fragColor.xyz = 1.-fragColor.xyz;

  gl_FragColor = fragColor;
}
