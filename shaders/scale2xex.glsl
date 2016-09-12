uniform vec4		u_param;
varying vec2		v_texCoord[5];

#ifdef FRAGMENT
uniform sampler	s_texture0;

void main()
{
	my_v3 E = textureColor(s_texture0, v_texCoord[2]).xyz;
	my_v3 D = textureColor(s_texture0, v_texCoord[1]).xyz;
	my_v3 F = textureColor(s_texture0, v_texCoord[3]).xyz;
	my_v3 H = textureColor(s_texture0, v_texCoord[4]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord[0]).xyz;
	my_v2 subTexel = fract(v_texCoord[2].xy);
	
	if(subTexel.y >= 0.5)
	{
		// adjust to E2 (swap B and H)
		my_v3 tmp = B; B = H; H = tmp;
	}
	if(subTexel.x >= 0.5)
	{
		// adjust to E1 or E3 (swap D and F)
		my_v3 tmp = F; F = D; D = tmp;
	}

	if(D == B && B != H && D != F)
 		E = D;

	gl_FragColor.xyz = E;
}

#else

attribute vec4		a_position;
attribute vec2		a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 offsetX = vec2(0.5/u_param.x, 0.0);
	my_v2 offsetY = vec2(0.0, 0.5/u_param.y);

	v_texCoord[0] = a_texCoord0 - offsetY;
	v_texCoord[1] = a_texCoord0 - offsetX;
	v_texCoord[2] = a_texCoord0;
	v_texCoord[3] = a_texCoord0 + offsetX;
	v_texCoord[4] = a_texCoord0 + offsetY;

	gl_Position = u_projection * a_position;
}

#endif
