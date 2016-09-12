varying vec2 v_texCoord[5];
uniform vec4		u_param;

#ifdef FRAGMENT

uniform sampler s_texture0;

void main()
{
	my_v3 E = textureColor(s_texture0, v_texCoord[0]).xyz;
	my_v3 D = textureColor(s_texture0, v_texCoord[1]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord[2]).xyz;
	my_v3 H = textureColor(s_texture0, v_texCoord[3]).xyz;
	my_v3 F = textureColor(s_texture0, v_texCoord[4]).xyz;

	my_v3 dt = vec3(1.0,1.0,1.0);
	float k1=dot(abs(D-F),dt)+0.001;
	float k2=dot(abs(H-B),dt)+0.001;

	gl_FragColor.xyz = (k1*(H+B)+k2*(D+F))/(2.0*(k1+k2));
}

#else

attribute vec4 a_position;
attribute vec2 a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 osx = vec2(0.5/u_param.x, 0.0);
	my_v2 osy = vec2(0.0, 0.5/u_param.y);
	gl_Position = u_projection * a_position;

	v_texCoord[0] = a_texCoord0;
	v_texCoord[1] = v_texCoord[0] - osx - osy;
	v_texCoord[2] = v_texCoord[0] + osx - osy;
	v_texCoord[3] = v_texCoord[0] - osx + osy;
	v_texCoord[4] = v_texCoord[0] + osx + osy;

}

#endif
