varying vec2 v_texCoord1[8];
varying vec2 v_texCoord2[8];
uniform vec4 u_param;

#ifdef FRAGMENT

uniform sampler s_texture0;

float getResult(float A,float B,float C,float D) {
	return float(A != C && A != D && B == C && B == D)-float(A == C && A == D);
}

void main()
{
/*
I E F J
G A B K
H C D L
M N O P
*/
	my_v2 p = fract(v_texCoord1[0].xy * u_param.xy);
	float q = 0.0;

	my_v3 A = textureColor(s_texture0, v_texCoord1[0]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord1[1]).xyz;
	my_v3 C = textureColor(s_texture0, v_texCoord1[2]).xyz;
	my_v3 D = textureColor(s_texture0, v_texCoord1[3]).xyz;
	my_v3 r = A;

	if (p.x>=0.5) {
	  if ((C==B) && (A!=D)) r = C;
	  else if ((A==D) && (C!=B)) r = A;
	  else if ((A==D) && (C==D) && (A!=B)) r = 0.5*(A+B);
	  else {
	    if (p.y<0.5) {
	      my_v3 M = textureColor(s_texture0, v_texCoord2[4]).xyz;
	      my_v3 N = textureColor(s_texture0, v_texCoord2[5]).xyz;
	      my_v3 O = textureColor(s_texture0, v_texCoord2[6]).xyz;
	      my_v3 P = textureColor(s_texture0, v_texCoord2[7]).xyz;
	      if ((B==D) && (D==N) && (C!=O) && (D!=M))
	        r = 0.25 * (3.0*D+C);
	      else if ((A==C) && (C==O) && (N!=D) && (C!=P))
	        r = 0.25 * (D+3.0*C);
	      else
	        r = 0.5 * (D+C);
	    } else {
	      my_v3 I = textureColor(s_texture0, v_texCoord2[0]).xyz;
	      my_v3 E = textureColor(s_texture0, v_texCoord1[4]).xyz;
	      my_v3 F = textureColor(s_texture0, v_texCoord1[5]).xyz;
	      my_v3 J = textureColor(s_texture0, v_texCoord2[1]).xyz;
	      if ((B==D) && (B==E) && (A!=F) && (B!=I))
	        r = 0.25 * (3.0*B+A);
	      else if ((A==C) && (A==F) && (E!=B) && (A!=J))
	        r = 0.25 * (B+3.0*A);
	      else
	        r = 0.5 * (A+B);
	    }
	  }
	} else if (p.y<0.5) {
	    my_v3 G = textureColor(s_texture0, v_texCoord1[6]).xyz;
	    my_v3 H = textureColor(s_texture0, v_texCoord1[7]).xyz;
	    my_v3 M = textureColor(s_texture0, v_texCoord2[4]).xyz;
	    my_v3 O = textureColor(s_texture0, v_texCoord2[6]).xyz;
	    if ((A==D) && (C!=B) && (G==A) && (A!=O)) 
	      r = 0.5 *(C+A);
	    else if ((A==H) && (B==A) && (G!=C) && (A!=M))
	      r = 0.5 *(C+A);
	    else
	      r = C;
	} else {
	    my_v3 G = textureColor(s_texture0, v_texCoord1[6]).xyz;
	    my_v3 H = textureColor(s_texture0, v_texCoord1[7]).xyz;
	    my_v3 I = textureColor(s_texture0, v_texCoord2[0]).xyz;
	    my_v3 F = textureColor(s_texture0, v_texCoord1[5]).xyz;
	    if ((C==B) && (A!=D) && (H==C) && (C!=F)) 
	      r = 0.5 *(C+A);
	    else if ((G==C) && (D==C) && (H!=A) && (C!=F))
	      r = 0.5 *(C+A);
	    else r=A;
	}

	gl_FragColor.xyz=r;
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
/*
 8  4  5  9
 6  0  1 10
 7  2  3 11
12 13 14 15
*/
	v_texCoord1[0] = a_texCoord0;
	v_texCoord1[1] = v_texCoord1[0] + osx;
	v_texCoord1[2] = v_texCoord1[0] - osy;
	v_texCoord1[3] = v_texCoord1[2] + osx;
	v_texCoord1[4] = v_texCoord1[0] + osy;
	v_texCoord1[5] = v_texCoord1[4] + osx;
	v_texCoord1[6] = v_texCoord1[0] - osx;
	v_texCoord1[7] = v_texCoord1[6] - osy;
	v_texCoord2[0] = v_texCoord1[6] + osy;
	v_texCoord2[1] = v_texCoord1[5] + osx;
	v_texCoord2[2] = v_texCoord1[1] + osx;
	v_texCoord2[3] = v_texCoord1[3] + osx;
	v_texCoord2[4] = v_texCoord1[7] - osy;
	v_texCoord2[5] = v_texCoord1[2] - osy;
	v_texCoord2[6] = v_texCoord1[3] - osy;
	v_texCoord2[7] = v_texCoord2[3] - osy;
}

#endif
