varying vec4 v_color;

#ifdef FRAGMENT

void main()
{
	gl_FragColor = v_color;
}

#else // VERT

attribute vec4		a_position;
attribute vec4		a_color;
uniform mat4		u_projection;

void main()
{
	gl_Position = u_projection * a_position;
	v_color = a_color;
}

#endif
