#version 120

varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec4 v_color;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;

void
main(void)
{
	vec4 v_tex0 = texture2D(u_texture0, v_texcoord0);
	vec4 v_tex1 = texture2D(u_texture1, v_texcoord1);
	vec4 col = v_tex0+v_tex1;
	col.a = v_tex0.a * v_tex1.a;
	gl_FragColor = v_color*col;
}
