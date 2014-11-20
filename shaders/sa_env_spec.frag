#version 120

varying vec2 v_texcoord0;
varying vec2 v_texcoord1;
varying vec2 v_texcoord2;
varying vec4 v_color;
varying vec4 v_envcolor;
varying vec4 v_speccolor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

void
main(void)
{
	vec4 v_tex0 = texture2D(texture0, v_texcoord0);
	vec4 v_tex1 = texture2D(texture1, v_texcoord1);
	vec4 v_tex2 = texture2D(texture2, v_texcoord2);

	gl_FragColor = v_tex0*v_color;
	gl_FragColor.rgb += (v_tex1*v_envcolor +
	                     v_tex2*v_speccolor).rgb;
}
