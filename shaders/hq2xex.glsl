uniform vec4		u_param;
varying vec2		v_texCoord[9];

#ifdef FRAGMENT
uniform sampler	s_texture0;

void main()
{
	const float mx = 0.325;
	const float k = -0.250;
	const float maxW = 0.25;
	const float minW =-0.05;
	const float lumAdd = 0.25;
	const my_v3 dt = vec3(1.0, 1.0, 1.0);

	my_v3 c00 = textureColor(s_texture0, v_texCoord[0]).rgb;
	my_v3 c10 = textureColor(s_texture0, v_texCoord[1]).rgb;
	my_v3 c20 = textureColor(s_texture0, v_texCoord[2]).rgb;
	my_v3 c01 = textureColor(s_texture0, v_texCoord[3]).rgb;
	my_v3 c11 = textureColor(s_texture0, v_texCoord[4]).rgb;
	my_v3 c21 = textureColor(s_texture0, v_texCoord[5]).rgb;
	my_v3 c02 = textureColor(s_texture0, v_texCoord[6]).rgb;
	my_v3 c12 = textureColor(s_texture0, v_texCoord[7]).rgb;
	my_v3 c22 = textureColor(s_texture0, v_texCoord[8]).rgb;

	float md1 = dot(abs(c00 - c22), dt);
	float md2 = dot(abs(c02 - c20), dt);

	float w1 = dot(abs(c22 - c11), dt) * md2;
	float w2 = dot(abs(c02 - c11), dt) * md1;
	float w3 = dot(abs(c00 - c11), dt) * md2;
	float w4 = dot(abs(c20 - c11), dt) * md1;

	float t1 = w1 + w3;
	float t2 = w2 + w4;
	float ww = max(t1, t2) + 0.0001;

	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);

	float lc1 = k / (0.12 * dot(c10 + c12 + c11, dt) + lumAdd);
	float lc2 = k / (0.12 * dot(c01 + c21 + c11, dt) + lumAdd);

	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, minW, maxW);
	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, minW, maxW);
	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, minW, maxW);
	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, minW, maxW);

	gl_FragColor.xyz = w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 + (1.0 - w1 - w2 - w3 - w4) * c11;
}

#else

attribute vec4		a_position;
attribute vec2		a_texCoord0;
uniform mat4		u_projection;

void main()
{
	my_v2 offsetDiag = vec2(0.5/u_param.x, 0.5/u_param.y);
	my_v2 offsetDiagNegX = vec2(-0.5/u_param.x, 0.5/u_param.y);
	my_v2 offsetX = vec2(0.5/u_param.x, 0.0);
	my_v2 offsetY = vec2(0.0, 0.5/u_param.y);
	v_texCoord[0] = a_texCoord0 - offsetDiag;
	v_texCoord[1] = a_texCoord0 - offsetY;
	v_texCoord[2] = a_texCoord0 - offsetDiagNegX;
	v_texCoord[3] = a_texCoord0 - offsetX;
	v_texCoord[4] = a_texCoord0;
	v_texCoord[5] = a_texCoord0 + offsetX;
	v_texCoord[6] = a_texCoord0 + offsetDiagNegX;
	v_texCoord[7] = a_texCoord0 + offsetY;
	v_texCoord[8] = a_texCoord0 + offsetDiag;
	gl_Position = u_projection * a_position;
}

#endif
