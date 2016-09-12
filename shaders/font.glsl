uniform vec4		u_param;
varying vec2		v_texCoord[5];

#ifdef FRAGMENT

uniform vec4		u_color;
uniform sampler2D	s_texture0;

void main()
{
	float E = texture2D(s_texture0, v_texCoord[0]).a;
	my_v4 origColor = E * u_color;
/*	my_v4 bl = vec4(0.4,0.4,0.4,1.0);
	float D = texture2D(s_texture0, v_texCoord[1]).a;
	float F = texture2D(s_texture0, v_texCoord[2]).a;
	float H = texture2D(s_texture0, v_texCoord[3]).a;
	float B = texture2D(s_texture0, v_texCoord[4]).a;

	float ua = 0.0;
	ua = mix(ua, 1.0, E);
	ua = mix(ua, 1.0, D);
	ua = mix(ua, 1.0, F);
	ua = mix(ua, 1.0, H);
	ua = mix(ua, 1.0, B);

	my_v4 underColor = bl * vec4(ua);

	gl_FragColor = underColor;
	gl_FragColor = mix(gl_FragColor, origColor, origColor.a);*/
	gl_FragColor = origColor;
}

#else // VERT

attribute vec4		a_position;
attribute vec2		a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 osx = vec2(1.0/u_param.x, 0.0);
	my_v2 osy = vec2(0.0, 1.0/u_param.y);

	gl_Position = u_projection * a_position;

	v_texCoord[0] = a_texCoord0;
	v_texCoord[1] = v_texCoord[0] - osx;
	v_texCoord[2] = v_texCoord[0] + osx;
	v_texCoord[3] = v_texCoord[0] - osy;
	v_texCoord[4] = v_texCoord[0] + osy;
}

#endif
