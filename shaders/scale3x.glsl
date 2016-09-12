varying vec2 v_texCoord[9];
uniform vec4		u_param;

#ifdef FRAGMENT

uniform sampler s_texture0;

/*
 A B C
 D E F
 G H I

 k0k1
 k2k3

E0E1E2
E3E4E5
E6E7E8
*/

void main()
{
	my_v2 p = fract(v_texCoord[0].xy);

	my_v3 D = textureColor(s_texture0, v_texCoord[1]).xyz;
	my_v3 F = textureColor(s_texture0, v_texCoord[2]).xyz;
	my_v3 H = textureColor(s_texture0, v_texCoord[3]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord[4]).xyz;
	my_v3 E = textureColor(s_texture0, v_texCoord[0]).xyz;
	my_v3 r = E;

	if (D == B && B != F && D != H) {						//=k0=
		if(p.x< 0.33		&& p.y>=0.66)										r = D;	// E0
		else if(p.x>=0.33&&p.x< 0.66	&& p.y>=0.66		&& E != textureColor(s_texture0, v_texCoord[6]).xyz)	r = B;  // E1
		else if(p.x< 0.33		&& p.y>=0.33&&p.y< 0.66	&& E != textureColor(s_texture0, v_texCoord[7]).xyz)	r = D;  // E3
	} else	if (B == F && B != D && F != H) {					//=k1=
		if(p.x>=0.66		&& p.y>=0.66)										r = F;	// E2
		else if(p.x>=0.33&&p.x< 0.66 && p.y>=0.66		&& E != textureColor(s_texture0, v_texCoord[5]).xyz)	r = B;  // E1
		else if(p.x>=0.66		&& p.y>=0.33&&p.y< 0.66	&& E != textureColor(s_texture0, v_texCoord[8]).xyz)	r = F;  // E5
	} else 	if (D == H && D != B && H != F) {					//=k2=
		if(p.x< 0.33		&& p.y< 0.33)										r = D;  // E6
		else if(p.x< 0.33		&& p.y>=0.33&&p.y< 0.66 && E != textureColor(s_texture0, v_texCoord[5]).xyz)	r = D;	// E3
		else if(p.x>=0.33&&p.x< 0.66 && p.y< 0.33		&& E != textureColor(s_texture0, v_texCoord[8]).xyz)	r = H;  // E7
	} else 	if (H == F && D != H && B != F) {					//=k3=
		if(p.x>=0.66		&& p.y< 0.33)										r = F;  // E8
		else if(p.x>=0.66		&& p.y>=0.33&&p.y< 0.66 && E != textureColor(s_texture0, v_texCoord[6]).xyz)	r = F;	// E5
		else if(p.x>=0.33&&p.x< 0.66 && p.y< 0.33		&& E != textureColor(s_texture0, v_texCoord[7]).xyz)	r = H;  // E7
	}

	gl_FragColor.xyz = r;
}

#else

attribute vec4 a_position;
attribute vec2 a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 osx = vec2(0.5/(u_param.x), 0.0);
	my_v2 osy = vec2(0.0, 0.5/(u_param.y));

	gl_Position = u_projection * a_position;

	v_texCoord[0] = a_texCoord0;			// E
	v_texCoord[1] = v_texCoord[0] - osx;		// D
	v_texCoord[2] = v_texCoord[0] + osx;		// F
	v_texCoord[3] = v_texCoord[0]       - osy;	// H
	v_texCoord[4] = v_texCoord[0]       + osy;	// B
	v_texCoord[5] = v_texCoord[0] - osx + osy;	// A
	v_texCoord[6] = v_texCoord[0] + osx + osy;	// C
	v_texCoord[7] = v_texCoord[0] - osx - osy;	// G
	v_texCoord[8] = v_texCoord[0] + osx - osy;	// I
}

#endif
