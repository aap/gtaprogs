#version 120

attribute vec3 in_vertex;
attribute vec3 in_normal;
attribute vec2 in_texcoord0;
attribute vec4 in_color;
attribute vec4 in_weights;
attribute vec4 in_indices;

varying vec2 v_texcoord0;
//varying vec2 v_texcoord1;
varying vec4 v_color;

uniform int u_lightmodel;
uniform vec4 u_matcolor;
uniform vec4 u_ambcolor;
uniform vec3 u_lightdir;
uniform mat3 u_normalmat;
uniform mat4 pmat;
uniform mat4 mvmat;
uniform mat4 bonemats[64];

void
main(void)
{
	mat4 modelview = mvmat;
	if(in_weights[0] != 0.0){
		mat4 m = bonemats[int(in_indices[0])] * in_weights[0];
		for(int i = 1; i < 4 && in_weights[i] != 0.0; i++){
			m += bonemats[int(in_indices[i])] * in_weights[i];
		}
		modelview = mvmat * m;
	}

	gl_Position = pmat * modelview * vec4(in_vertex, 1.0);
//	vec3 u = normalize(vec3(mvmat * vec4(in_vertex, 1.0)));
////	vec3 n = normalize(u_normalmat * in_normal);
//	vec3 n = normalize(mat3(modelview) * in_normal);
//	vec3 r = reflect(u, n);
//	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
//	v_texcoord1.s = r.x/m + 0.5;
//	v_texcoord1.t = r.y/m + 0.5;

	v_texcoord0 = in_texcoord0;
	if(u_lightmodel == 0){	/* dynamic, normals */
//		vec3 N = u_normalmat * in_normal;
		vec3 N = mat3(modelview) * in_normal;
		float L = max(0.0, dot(N, -u_lightdir));
		vec4 lightval = vec4(L, L, L, 1.0);
		v_color = u_matcolor*lightval + u_matcolor*u_ambcolor;
		v_color.a = u_matcolor.a;
	}else{			/* prelit */
		v_color = in_color + u_matcolor*u_ambcolor;
		v_color.a = in_color.a * u_matcolor.a;
	}
//	v_color = clamp(v_color, 0.0, 1.0);
}
