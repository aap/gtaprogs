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
uniform mat4 pmat;
uniform mat4 mvmat;
uniform float reflMult;
uniform float specMult;

void
main(void)
{
	gl_Position = pmat * mvmat * vec4(in_vertex, 1.0);

	v_texcoord0 = in_texcoord0;
	v_texcoord1 = in_texcoord1;

	vec3 V = u_lightdir*vec3(-1, 1, -1);
	vec3 U = vec3(V.x+1, V.y+1, V.z)*0.5;
	vec3 n = mat3(mvmat)*in_normal*vec3(-1, 1, -1);
	vec3 st = U - n*dot(n, V);
	v_texcoord2.st = st.xy;

	// actually in_color
	v_color = vec4(0, 0, 0, 1);
	// lambert
	float l = max(0.0, dot(mat3(mvmat)*in_normal, -u_lightdir));
	v_color.rgb += l*vec3(0.5804683);
	// ambient
	v_color.rgb += vec3(0.40449339);
	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= u_matcolor;

////	vec3 N = u_normalmat * in_normal;
//	vec3 N = mat3(mvmat) * in_normal;
//	float L = max(0.0, dot(N, -u_lightdir));
//	vec4 lightval = vec4(L, L, L, 1.0);
//	v_color = u_matcolor*lightval + u_matcolor*u_ambcolor;
////	v_color = u_matcolor*lightval;
//	v_color.a = u_matcolor.a;

	v_envcolor = vec4(192.0/128.0)*reflMult*0.5827;

	if(st.z < 0){
		v_speccolor = vec4(96.0/128.0)*specMult*0.5827;
		v_speccolor.a = v_color.a;
	}else
		v_speccolor = vec4(0.0, 0.0, 0.0, 1.0);

}
