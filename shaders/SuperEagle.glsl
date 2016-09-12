varying vec2	v_texCoord1[8];
varying vec2	v_texCoord2[8];
uniform vec4	u_param;


#ifdef FRAGMENT

uniform sampler s_texture0;

float getResult(vec3 A,vec3 B,vec3 C,vec3 D) {
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

	my_v3 A = textureColor(s_texture0, v_texCoord1[0]).xyz;
	my_v3 B = textureColor(s_texture0, v_texCoord1[1]).xyz;
	my_v3 C = textureColor(s_texture0, v_texCoord1[2]).xyz;
	my_v3 D = textureColor(s_texture0, v_texCoord1[3]).xyz;
	my_v3 r = A;
	
	if ((C==B) && (A!=D)) {
	  if ((p.x>0.5&&p.y>0.5)||(p.x<0.5&&p.y<0.5)) {
	    r = C;
	  } else {
	    my_v3 H = textureColor(s_texture0, v_texCoord1[7]).xyz;
	    my_v3 K = textureColor(s_texture0, v_texCoord2[2]).xyz;
	    my_v3 N = textureColor(s_texture0, v_texCoord2[5]).xyz;
	    my_v3 F = textureColor(s_texture0, v_texCoord1[5]).xyz;
	    if (((H==C) && (B==K)) || ((C==N) && (B==F))) {
	      if (p.x<0.5) {
	        r = 0.25 * (3.0*C+A);
	      } else {
	        r = 0.25 * (3.0*C+D);
	      }
	    } else {
	      if (p.x<0.5) {
	        r = 0.5 * (B+A);
	      } else {
	        r = 0.5 * (C+D);
	      }
	    }
	  }
	} else if ((A==D) && (C!=B)) {
	  if ((p.x>0.5&&p.y<0.5)||(p.x<0.5&&p.y>0.5)) {
	    r = A;
	  } else {
	    my_v3 E = textureColor(s_texture0, v_texCoord1[4]).xyz;
	    my_v3 O = textureColor(s_texture0, v_texCoord2[6]).xyz;
	    my_v3 G = textureColor(s_texture0, v_texCoord1[6]).xyz;
	    my_v3 L = textureColor(s_texture0, v_texCoord2[3]).xyz;
	    if (((E==A) && (D==O)) || ((G==A) && (D==L))) {
	      if (p.x>0.5) {
	        r = 0.25 * (3.0*A+B);
	      } else {
	        r = 0.25 * (3.0*A+C);
	      }
	    } else {
	      if (p.x>0.5) {
	        r = 0.5 * (B+A);
	      } else {
	        r = 0.5 * (C+D);
	      }
	    }
	  }
	} else if ((A==D) && (C==B) && (A!=B)) {
	  float s  = 0.0;
	  my_v3 H = textureColor(s_texture0, v_texCoord1[7]).xyz;
	  my_v3 N = textureColor(s_texture0, v_texCoord2[5]).xyz;
	  my_v3 E = textureColor(s_texture0, v_texCoord1[4]).xyz;
	  my_v3 G = textureColor(s_texture0, v_texCoord1[6]).xyz;
	  my_v3 O = textureColor(s_texture0, v_texCoord2[6]).xyz;
	  my_v3 L = textureColor(s_texture0, v_texCoord2[3]).xyz;
	  my_v3 F = textureColor(s_texture0, v_texCoord1[5]).xyz;
	  my_v3 K = textureColor(s_texture0, v_texCoord2[2]).xyz;
	  s += getResult(B,A,H,N);
	  s += getResult(B,A,G,E);
	  s += getResult(B,A,O,L);
	  s += getResult(B,A,F,K);
	  if (s>0.0) {
	    if ((p.x>0.5&&p.y<=0.5)||(p.x<=0.5&&p.y>0.5)) {
	      r = 0.5 * (A+B);
	    } else {
	      r = C;
	    }
	  } else if (s<0.0) {
	    if ((p.x>0.5&&p.y<=0.5)||(p.x<=0.5&&p.y>0.5)) {
	      r = A;
	    } else {
	      r = 0.5 * (A+B);
	    }
	  } else {
	    if ((p.x>0.5&&p.y<=0.5)||(p.x<=0.5&&p.y>0.5)) {
	      r = A;
	    } else {
	      r = C;
	    }
	  }
	} else {
	  if ((A==C) || (B==D)) {
	    if (p.x<=0.5 && p.y>0.5) 		r = A;
	    else if (p.x<=0.5 && p.y<=0.5)	r = C;
	    else if (p.x> 0.5 && p.y>0.5)	r = B;
	    else				r = D;
	  } else {
	    if (p.x<=0.5 && p.y>0.5) 		r = 0.25*(3.0*A+B);
	    else if (p.x<=0.5 && p.y<=0.5)	r = 0.25*(3.0*C+D);
	    else if (p.x> 0.5 && p.y>0.5)	r = 0.25*(3.0*B+A);
	    else				r = 0.25*(3.0*D+C);
	  }
	}

	gl_FragColor.xyz=r;
}

#else

attribute vec4 a_position;
attribute vec2 a_texCoord0;
uniform   mat4 u_projection;

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
