#version 120

varying vec2 v_texcoord0;
varying vec4 v_color;

uniform sampler2D texture0;

void
main(void)
{
	vec4 v_tex0 = texture2D(texture0, v_texcoord0);
	gl_FragColor = v_color * v_tex0;
}
