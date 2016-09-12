varying vec2		v_texCoord0;

#ifdef FRAGMENT

uniform vec4		u_color;
uniform sampler2D	s_texture0;

void main()
{
	gl_FragColor = texture2D(s_texture0, v_texCoord0) * u_color;
}

#else // VERT

attribute vec4		a_position;
attribute vec2		a_texCoord0;
uniform mat4		u_projection;

void main()
{
	gl_Position = u_projection * a_position;
	v_texCoord0 = a_texCoord0;
}

#endif
