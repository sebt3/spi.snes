varying vec2 v_texCoord[5];
uniform vec4		u_param;

#ifdef FRAGMENT
uniform sampler s_texture0;

void main()
{
	mm_v3 c11 = textureColor(s_texture0, v_texCoord[0]).xyz; 
	mm_v3 c00 = textureColor(s_texture0, v_texCoord[1]).xyz; 
	mm_v3 c20 = textureColor(s_texture0, v_texCoord[2]).xyz; 
	mm_v3 c22 = textureColor(s_texture0, v_texCoord[3]).xyz; 
	mm_v3 c02 = textureColor(s_texture0, v_texCoord[4]).xyz; 
	mm_v3 dt  = vec3(1.0,1.0,1.0);

	float m1=dot(abs(c00-c22),dt);
	float m2=dot(abs(c02-c20),dt);
	float w2=dot(abs(c02-c11),dt);
	float w1=dot(abs(c22-c11),dt);
	float w3=dot(abs(c00-c11),dt);
	float w4=dot(abs(c20-c11),dt);

	m1= min(w1,m1);m2= min(w2,m2);
	w1*=m2;w2*=m1; w3*=m2; w4*=m1;

	gl_FragColor.xyz=(w1*c00+w2*c20+w3*c22+w4*c02+0.001*c11)/(w1+w2+w3+w4+0.001);
}

#else

attribute vec4 a_position;
attribute vec2 a_texCoord0;
uniform mat4		u_projection;

void main()
{
	vec2 osx = vec2(0.5/u_param.x, 0.0);
	vec2 osy = vec2(0.0, 0.5/u_param.y);

	gl_Position = u_projection * a_position;

	v_texCoord[0] = a_texCoord0;
	v_texCoord[1] = v_texCoord[0] - osx + osy;
	v_texCoord[2] = v_texCoord[0] - osx - osy;
	v_texCoord[3] = v_texCoord[0] + osx - osy;
	v_texCoord[4] = v_texCoord[0] + osx + osy;
}

#endif
