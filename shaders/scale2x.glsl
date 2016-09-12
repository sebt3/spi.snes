uniform vec4		u_param;
varying vec2		v_texCoord[5];
varying vec2		pos;

#ifdef FRAGMENT
uniform sampler	s_texture0;

void main()
{
/*
 A B C
 D E F
 G H I
*/
	my_v3 E = textureColor(s_texture0, v_texCoord[0]).xyz;
	my_v3 D = textureColor(s_texture0, v_texCoord[1]).xyz;
	my_v3 F = textureColor(s_texture0, v_texCoord[2]).xyz;
	my_v3 H = textureColor(s_texture0, v_texCoord[3]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord[4]).xyz;

	if ((D - F) * (H - B) == vec3(0.0)) {
		gl_FragColor.xyz = E;
	} else {
		my_v2 p = fract(pos);
		my_v3 tmp1 = p.x < 0.5 ? D : F;
		my_v3 tmp2 = p.y < 0.5 ? H : B;
		gl_FragColor.xyz = ((tmp1 - tmp2) != vec3(0.0)) ? E : tmp1;
	}
}

#else

attribute vec4		a_position;
attribute vec2		a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 osx = vec2(0.5/u_param.x, 0.0);
	my_v2 osy = vec2(0.0, 0.5/u_param.y);

	gl_Position = u_projection * a_position;

	v_texCoord[0] = a_texCoord0;
	v_texCoord[1] = v_texCoord[0] - osx;
	v_texCoord[2] = v_texCoord[0] + osx;
	v_texCoord[3] = v_texCoord[0] - osy;
	v_texCoord[4] = v_texCoord[0] + osy;
	pos  	      = v_texCoord[0].xy * u_param.xy;
}

#endif
