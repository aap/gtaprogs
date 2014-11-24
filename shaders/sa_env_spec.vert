#version 120

attribute vec3 in_vertex;
attribute vec3 in_normal;
attribute vec2 in_texcoord0;
attribute vec2 in_texcoord1;

varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec2 v_texcoord2;
varying vec4 v_color;
varying vec4 v_envcolor;
varying vec4 v_speccolor;

uniform vec4 u_matcolor;
uniform vec4 u_ambcolor;
uniform vec3 u_lightdir;
uniform mat3 u_normalmat;
uniform mat4 u_pmat;
uniform mat4 u_mvmat;
uniform float u_reflMult;
uniform vec4 u_envXform;
uniform float u_specMult;

void
main(void)
{
	gl_Position = u_pmat * u_mvmat * vec4(in_vertex, 1.0);

	v_texcoord0 = in_texcoord0;


	mat3 M = mat3(-0.032576822, 0.99946898, 0,
	              -0.99946761, -0.032578085, 0,
	              0.0017960054, -0.00068164448, 0);
	vec2 tmp = (M*in_normal).xy - u_envXform.xy;	// offset
	v_texcoord1.s = tmp.x + in_texcoord1.s;
	v_texcoord1.t = tmp.y*u_envXform.y + in_texcoord1.t;
	v_texcoord1 *= -u_envXform.zw;	// scale

	vec3 V = u_lightdir*vec3(-1, 1, -1);
	vec3 U = vec3(V.x+1, V.y+1, V.z)*0.5;
	vec3 n = mat3(u_mvmat)*in_normal*vec3(-1, 1, -1);
	vec3 st = U - n*dot(n, V);
	v_texcoord2.st = st.xy;

	float diffcol = 0.5804683;			// @ 0x3d4.xyz
	float magic2 = magic*1.0039216688675805;	// @ 0x3cd.w

	// should actually be in_color
	v_color = vec4(0, 0, 0, 1);
	// lambert
	float l = max(0.0, dot(mat3(u_mvmat)*in_normal, -u_lightdir));
	v_color.rgb += l*vec3(diffcol);
	// ambient
	v_color.rgb += u_ambcolor.rgb*diffcol;
	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= u_matcolor;

	v_envcolor = vec4(192.0/128.0)*u_reflMult*magic2;

	if(st.z < 0){
		v_speccolor = vec4(96.0/128.0)*u_specMult*magic2;
		v_speccolor.a = v_color.a;
	}else
		v_speccolor = vec4(0.0, 0.0, 0.0, 1.0);

}
