#ifdef FRAGMENT

uniform vec4 u_color;

void main()
{
	gl_FragColor = u_color;
}

#else // VERT

uniform mat4		u_projection;
attribute vec4		a_position;

void main()
{
	gl_Position = u_projection * a_position;
}

#endif
