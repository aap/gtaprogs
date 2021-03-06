#version 120

attribute vec3 in_vertex;
attribute vec2 in_texcoord0;
attribute vec4 in_color;

varying vec2 v_texcoord0;
varying vec4 v_color;

uniform mat4 u_pmat;
uniform mat4 u_mvmat;

void
main(void)
{
	gl_Position = u_pmat * u_mvmat * vec4(in_vertex, 1.0);
	v_texcoord0 = in_texcoord0;
	v_color = in_color;
}
