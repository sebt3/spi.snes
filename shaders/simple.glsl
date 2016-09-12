varying vec2		v_texCoord0;

#ifdef FRAGMENT

uniform sampler		s_texture0;
uniform vec4		u_param;

void main()
{
	gl_FragColor = textureColor(s_texture0, v_texCoord0);
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
