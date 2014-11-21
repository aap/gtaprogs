#version 120

attribute vec3 in_vertex;
attribute vec3 in_normal;
attribute vec2 in_texcoord0;

varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec4 v_color;

uniform vec4 u_matcolor;
uniform vec4 u_ambcolor;
uniform vec3 u_lightdir;
uniform mat3 u_normalmat;
uniform mat4 u_pmat;
uniform mat4 u_mvmat;

void
main(void)
{
	gl_Position = u_pmat * u_mvmat * vec4(in_vertex, 1.0);

	vec3 u = normalize(vec3(u_mvmat * vec4(in_vertex, 1.0)));
//	vec3 n = normalize(u_normalmat * in_normal);
	vec3 n = normalize(mat3(u_mvmat) * in_normal);
	vec3 r = reflect(u, n);
	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
	v_texcoord1.s = r.x/m + 0.5;
	v_texcoord1.t = r.y/m + 0.5;

	v_texcoord0 = in_texcoord0;

//	vec3 N = u_normalmat * in_normal;
	vec3 N = mat3(u_mvmat) * in_normal;
	float L = max(0.0, dot(N, -u_lightdir));
	vec4 lightval = vec4(L, L, L, 1.0);
	v_color = u_matcolor*lightval + u_matcolor*u_ambcolor;
	v_color.a = u_matcolor.a;
}
