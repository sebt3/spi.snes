#include <math.h>
#include "filters.h"
#include "rgroup.h"
#include "engine.h"

static unsigned short squareIndex[6] = { 0, 1, 2, 2, 3, 0 };

#if defined(USE_SHADER)
static menu_Shader*	menu_Shader_color;
static menu_Shader*	menu_Shader_colors;
static menu_Shader*	menu_Shader_font;
static menu_Shader*	menu_Shader_texture;
static menu_Shader*	menu_Shader_filters[SHADER_MAX_ID];

static GLuint		cur_prgm;
#endif
static Eina_Bool	use_tex0;
static Eina_Bool	use_texPBO;
static Eina_Bool	use_blend;
static GLuint		pboID;
// private functions
void		menu_Texture_useTex0(menu_Texture* t);
menu_Segment*	menu_RGroup_Segment_new(menu_RGroup* m, const uint16_t d, const int cnt, const unsigned short* c);
void		menu_RGroup_Segment_free(menu_RGroup* m, menu_Segment* s);

/***********************************************************************************
 *				Shaders
 ***********************************************************************************/
#if defined(USE_SHADER)

void		menu_Shader_useProgram(GLuint p) {
	if (cur_prgm == p)	return;
	cur_prgm = p;
	GL_CHECK(glUseProgram(p));
}

GLuint		menu_Shader_loadFile(char *filename, GLint type) {
	GLint iStatus;
	char *aStrings[1] = { NULL };
#ifdef HAVE_GLES
	char *defs1 = 	"#ifdef GL_IMG_texture_stream2";
	char *defs2 = 	"#extension GL_IMG_texture_stream2 : enable";
	char *defs3 = 	"#endif";
	char *defs4 = 	"#define sampler lowp samplerStreamIMG";
	char *defs5 = 	"#define textureColor textureStreamIMG";
	if (!menu_havePBO) {
		defs1 = defs2 = defs3 = "";
		defs4  = "#define sampler sampler2D";
		defs5  = "#define textureColor texture2D";
	}
#endif
	int added_len = 0;

	FILE *f = fopen(filename, "r");
	if(f == NULL) {
		fprintf(stderr, "Error: Cannot read file '%s'\n", filename);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	aStrings[0] = (char*)calloc(len+1024, sizeof(char));
	if(aStrings[0] == (char *)NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}
	switch(type) {
	case GL_VERTEX_SHADER:
		added_len = sprintf(aStrings[0], "%s\n%s\n%s\n%s\n%s\n#define VERTEX\n", 
#ifdef HAVE_GLES
					"#define GLSLES",
					"#define my_v2 lowp vec2",
					"#define my_v3 lowp vec3",
					"#define mm_v3 mediump vec3",
					"#define my_v4 lowp vec4"
#else
					"#version 130",
					"#define my_v2 vec2",
					"#define mm_v3 vec3",
					"#define my_v3 vec3",
					"#define my_v4 vec4"
#endif
			);

	break;
	case GL_FRAGMENT_SHADER:
		added_len = sprintf(aStrings[0], "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n#define FRAGMENT\n", 
#ifdef HAVE_GLES
					"#define GLSLES",
					"#define my_v2 lowp vec2",
					"#define my_v3 lowp vec3",
					"#define mm_v3 mediump vec3",
					"#define my_v4 lowp vec4",
					"precision mediump float;",
					defs1,defs2,defs3,defs4,defs5
#else
					"#version 130",
					"#define my_v2 vec2",
					"#define my_v3 vec3",
					"#define mm_v3 vec3",
					"#define my_v4 vec4",
					"precision mediump float;",
					"",	"",	"",
					"#define sampler mediump sampler2D",
					"#define textureColor texture2D"
#endif
			);
	break;
	}
	fread((void*)(aStrings[0]+added_len), sizeof(char), len, f);
	aStrings[0][len+added_len] = '\0';
	fclose(f);


	/* Create shader and load into GL. */
	GLuint pShader = glCreateShader(type);
	GL_CHECK(glShaderSource(pShader, 1, (const char**)aStrings, NULL));

	/* Clean up shader source. */
	free((void *)(aStrings[0]));
	aStrings[0] = NULL;

	/* Try compiling the shader. */
	GL_CHECK(glCompileShader(pShader));
	GL_CHECK(glGetShaderiv(pShader, GL_COMPILE_STATUS, &iStatus));

	if(iStatus != GL_TRUE) {
		GLint infoLen = 0;
		glGetShaderiv(pShader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char* infoLog = (char*)malloc(infoLen);
			char *shtype;
			if (type== GL_FRAGMENT_SHADER)
			  shtype = "fragment";
			else
			  shtype = "vertex";
			glGetShaderInfoLog(pShader, infoLen, NULL, infoLog);
			printf("%s(%s) Compilation failed : %s\n", filename, shtype, infoLog);
			free(infoLog);
		}
		return -1;
	}
	return pShader;
}

menu_Shader*	menu_Shader_new(char *filename) {
	Uint32 tmp=SDL_GetTicks();
	char *tx;
#ifdef HAVE_GLES
	GLint iStatus;
#endif
	menu_Shader* ret	= calloc(1, sizeof(menu_Shader));
	ret->name		= calloc(20, sizeof(char));
	ret->prgm		= glCreateProgram();
	ret->vertx		= menu_Shader_loadFile(filename, GL_VERTEX_SHADER);
	ret->fragm		= menu_Shader_loadFile(filename, GL_FRAGMENT_SHADER);
	strcpy((char*)ret->name, filename+8);
	tx 			= strchr(ret->name, '.');
	*tx			= 0;
	GL_CHECK(glAttachShader(ret->prgm, ret->vertx));
	GL_CHECK(glAttachShader(ret->prgm, ret->fragm));
	GL_CHECK(glLinkProgram(ret->prgm));
	menu_Shader_useProgram(ret->prgm);
#ifdef HAVE_GLES
	GL_CHECK(glGetProgramiv(ret->prgm, GL_LINK_STATUS, &iStatus));
	if(iStatus != GL_TRUE) {
		GLint infoLen = 0;
		glGetProgramiv(ret->prgm, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char* infoLog = (char*)malloc(infoLen);
			glGetProgramInfoLog(ret->prgm, infoLen, NULL, infoLog);
			printf("%s Link failed : %s\n", filename, infoLog);
			free(infoLog);
		}
		return NULL;
	}
#endif
	printf("Compiled %s in %dms\n", filename, SDL_GetTicks() - tmp);
	return ret;
}

void		menu_Shader_free(menu_Shader* s) {
	GL_CHECK(glDetachShader(s->prgm, s->vertx));
	GL_CHECK(glDeleteShader(s->vertx));
	GL_CHECK(glDetachShader(s->prgm, s->fragm));
	GL_CHECK(glDeleteShader(s->fragm));
	GL_CHECK(glDeleteProgram(s->prgm));
}

// Variing colors management

void	menu_Shader_colors_draw(menu_Shader* s, menu_RGroup* r) {
	menu_Shader_useProgram(s->prgm);
	menu_Texture_useTex0(NULL);
	GL_CHECK(glVertexAttribPointer(s->a_position, 3, GL_FLOAT, GL_FALSE, 0, r->dots));
	GL_CHECK(glVertexAttribPointer(s->a_color,    4, GL_FLOAT, GL_FALSE, 0, r->cols));
	GL_CHECK(glUniformMatrix4fv(s->u_projection, 1, GL_FALSE, menu_Projection));
	GL_CHECK(glDrawElements(r->mode, r->indexCur, GL_UNSIGNED_SHORT, r->indexData));
}

menu_Shader*	menu_Shader_new_colors() {
	menu_Shader* ret	= menu_Shader_new("shaders/colors.glsl");
	ret->a_position		= glGetAttribLocation(ret->prgm,  "a_position");
	ret->a_color		= glGetAttribLocation(ret->prgm,  "a_color");
	ret->u_projection	= glGetUniformLocation(ret->prgm, "u_projection");
	ret->draw		= menu_Shader_colors_draw;
	GL_CHECK(glEnableVertexAttribArray(ret->a_position));
	GL_CHECK(glEnableVertexAttribArray(ret->a_color));
	return ret;
}
menu_Shader*	menu_Shader_get_colors() {
	return	menu_Shader_colors;
}

// Single color management

void	menu_Shader_color_draw(menu_Shader* s, menu_RGroup* r) {
	menu_Shader_useProgram(s->prgm);
	menu_Texture_useTex0(NULL);
	GL_CHECK(glVertexAttribPointer(s->a_position, 3, GL_FLOAT, GL_FALSE, 0, r->dots));
	GL_CHECK(glUniform4f(s->u_color, r->col.t[0], r->col.t[1], r->col.t[2], r->col.t[3]));
	GL_CHECK(glUniformMatrix4fv(s->u_projection, 1, GL_FALSE, menu_Projection));
	GL_CHECK(glDrawElements(r->mode, r->indexCur, GL_UNSIGNED_SHORT, r->indexData));
}

menu_Shader*	menu_Shader_new_color() {
	menu_Shader* ret	= menu_Shader_new("shaders/color.glsl");
	ret->a_position		= glGetAttribLocation(ret->prgm,  "a_position");
	ret->u_color		= glGetUniformLocation(ret->prgm, "u_color");
	ret->u_projection	= glGetUniformLocation(ret->prgm, "u_projection");
	ret->draw		= menu_Shader_color_draw;
	GL_CHECK(glEnableVertexAttribArray(ret->a_position));
	return ret;
}
menu_Shader*	menu_Shader_get_color() {
	return	menu_Shader_color;
}

// Font management

void	menu_Shader_font_draw(menu_Shader* s, menu_RGroup* r) {
	menu_Shader_useProgram(s->prgm);
	menu_Texture_useTex0(r->t);
	menu_Texture_bind(r->t);
	GL_CHECK(glVertexAttribPointer(s->a_position, 3, GL_FLOAT, GL_FALSE, 0, r->dots));
	GL_CHECK(glVertexAttribPointer(s->a_texCoord0,2, GL_FLOAT, GL_FALSE, 0, r->tex));
	GL_CHECK(glUniform4f(s->u_param, r->t->w, r->t->h, r->t->orig_w, r->t->orig_h));
	GL_CHECK(glUniform4fv(s->u_color, 1, r->col.t));
	GL_CHECK(glUniformMatrix4fv(s->u_projection, 1, GL_FALSE, menu_Projection));
	GL_CHECK(glDrawElements(r->mode, r->indexCur, GL_UNSIGNED_SHORT, r->indexData));
}

menu_Shader*	menu_Shader_new_font() {
	menu_Shader* ret	= menu_Shader_new("shaders/font.glsl");
	ret->a_position		= glGetAttribLocation(ret->prgm,  "a_position");
	ret->a_texCoord0	= glGetAttribLocation(ret->prgm,  "a_texCoord0");
	ret->u_color		= glGetUniformLocation(ret->prgm, "u_color");
	ret->u_param		= glGetUniformLocation(ret->prgm, "u_param");
	ret->u_projection	= glGetUniformLocation(ret->prgm, "u_projection");
	ret->draw		= menu_Shader_font_draw;
	GL_CHECK(glEnableVertexAttribArray(ret->a_position));
	GL_CHECK(glEnableVertexAttribArray(ret->a_texCoord0));
	return ret;
}
menu_Shader*	menu_Shader_get_font() {
	return	menu_Shader_font;
}

// Textures management

void	menu_Shader_texture_draw(menu_Shader* s, menu_RGroup* r) {
	menu_Shader_useProgram(s->prgm);
	menu_Texture_useTex0(r->t);
	menu_Texture_update(r->t);
	GL_CHECK(glVertexAttribPointer(s->a_position, 3, GL_FLOAT, GL_FALSE, 0, r->dots));
	GL_CHECK(glVertexAttribPointer(s->a_texCoord0,2, GL_FLOAT, GL_FALSE, 0, r->tex));
	GL_CHECK(glUniform4fv(s->u_color, 1, r->col.t));
	GL_CHECK(glUniformMatrix4fv(s->u_projection, 1, GL_FALSE, menu_Projection));
	GL_CHECK(glDrawElements(r->mode, r->indexCur, GL_UNSIGNED_SHORT, r->indexData));
}

menu_Shader*	menu_Shader_new_texture() {
	menu_Shader* ret	= menu_Shader_new("shaders/texture.glsl");
	ret->a_position		= glGetAttribLocation(ret->prgm,  "a_position");
	ret->a_texCoord0	= glGetAttribLocation(ret->prgm,  "a_texCoord0");
	ret->u_projection	= glGetUniformLocation(ret->prgm, "u_projection");
	ret->u_color		= glGetUniformLocation(ret->prgm, "u_color");
	ret->draw		= menu_Shader_texture_draw;
	GL_CHECK(glEnableVertexAttribArray(ret->a_position));
	GL_CHECK(glEnableVertexAttribArray(ret->a_texCoord0));
	return ret;
}
menu_Shader*	menu_Shader_get_texture() {
	return	menu_Shader_texture;
}

// Filters management

void	menu_Shader_filter_draw(menu_Shader* s, menu_RGroup* r) {
	menu_Shader_useProgram(s->prgm);
	menu_Texture_useTex0(r->t);
	menu_Texture_update(r->t);
	GL_CHECK(glVertexAttribPointer(s->a_position, 3, GL_FLOAT, GL_FALSE, 0, r->dots));
	GL_CHECK(glVertexAttribPointer(s->a_texCoord0,2, GL_FLOAT, GL_FALSE, 0, r->tex));
	GL_CHECK(glUniform4f(s->u_param, (float)r->t->w, (float)r->t->h, (float)r->t->orig_w, (float)r->t->orig_h));
	GL_CHECK(glUniformMatrix4fv(s->u_projection, 1, GL_FALSE, menu_Projection));
	GL_CHECK(glDrawElements(r->mode, r->indexCur, GL_UNSIGNED_SHORT, r->indexData));
}

menu_Shader*	menu_Shader_new_filter(char *filename) {
	menu_Shader* ret	= menu_Shader_new(filename);
	ret->a_position		= glGetAttribLocation(ret->prgm,  "a_position");
	ret->a_texCoord0	= glGetAttribLocation(ret->prgm,  "a_texCoord0");
	ret->u_param		= glGetUniformLocation(ret->prgm, "u_param");
	ret->u_projection	= glGetUniformLocation(ret->prgm, "u_projection");
	ret->draw		= menu_Shader_filter_draw;
	GL_CHECK(glEnableVertexAttribArray(ret->a_position));
	GL_CHECK(glEnableVertexAttribArray(ret->a_texCoord0));
	return ret;
}

menu_Shader*	menu_Shader_get_filter(uint16_t id) {
	if (id>=SHADER_MAX_ID) return NULL;
	return	menu_Shader_filters[id];
}
#endif

void	menu_Shader_init() {
	use_blend		= EINA_FALSE;
	use_tex0		= EINA_FALSE;
	use_texPBO		= EINA_FALSE;
	pboID			= 0;
#if defined(USE_SHADER)
	cur_prgm		= 255;
	menu_Shader_color	= menu_Shader_new_color();
	menu_Shader_colors	= menu_Shader_new_colors();
	menu_Shader_font	= menu_Shader_new_font();
	menu_main_Loading("Loading texture shader...", 2);
	menu_Shader_texture	= menu_Shader_new_texture();
	menu_main_Loading("Loading simple Stream shader...", 3);
	menu_Shader_filters[0]	= menu_Shader_new_filter("shaders/simple.glsl");
	menu_main_Loading("Loading scale2x Stream shader...", 4);
	menu_Shader_filters[1]	= menu_Shader_new_filter("shaders/scale2x.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 5);
	menu_Shader_filters[2]	= menu_Shader_new_filter("shaders/2xSal.glsl");
#ifndef HAVE_GLES
	menu_main_Loading("Loading 2xSal Stream shader...", 6);
	menu_Shader_filters[3]	= menu_Shader_new_filter("shaders/scale2xex.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 7);
	menu_Shader_filters[4]	= menu_Shader_new_filter("shaders/hq2xex.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 8);
	menu_Shader_filters[5]	= menu_Shader_new_filter("shaders/scale3x.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 9);
	menu_Shader_filters[6]	= menu_Shader_new_filter("shaders/Super2xSai.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 10);
	menu_Shader_filters[7]	= menu_Shader_new_filter("shaders/SuperEagle.glsl");
	menu_main_Loading("Loading 2xSal Stream shader...", 11);
	menu_Shader_filters[8]	= menu_Shader_new_filter("shaders/2xSai.glsl");
#endif
#endif
}


/***********************************************************************************
 *				Segments
 ***********************************************************************************/


static void _menu_Segment_resetMm(menu_Segment* ret) {
	uint16_t mxv  = (1<<15)-1;
	menu_vector_set(ret->min, 0,  mxv,  mxv,  mxv);
	menu_vector_set(ret->max, 0, -mxv, -mxv, -mxv);
}

menu_Segment*	menu_Segment_new(const uint16_t d, menu_RGroup* rg) {
	menu_Segment* ret	= calloc(1, sizeof(menu_Segment));
	ret->dotsCnt		= d;
	ret->r			= rg;
	ret->dots		= NULL;
	ret->cols		= NULL;
	ret->tex		= NULL;
	ret->indexLen		= 0;
	ret->indexData		= NULL;
	_menu_Segment_resetMm(ret);
	return ret;
}

void		menu_Segment_free(menu_Segment* s) {
	// Dont free the texture here as they are shared
	free(s);
}


void		menu_Segment_dot_set(menu_Segment* s, const int id, const menu_vector p) {
	int i;
	if (s->dots == NULL)
		s->dots = calloc(s->dotsCnt, sizeof(menu_vector));
	for(i=0;i<3;i++) {
		s->dots[3*id+i] = p[i];
		if(s->min[i]>p[i]) s->min[i]=p[i];
		if(s->max[i]<p[i]) s->max[i]=p[i];
	}
}

void		menu_Segment_col_set(menu_Segment* s, const int id, const menu_color c) {
	if (s->cols == NULL)
		s->cols = calloc(s->dotsCnt, sizeof(menu_color));
	menu_color_cpy(s->cols, id, c);
	if (c.t[3]<1.0)
		s->r->alpha	= EINA_TRUE;
}

void		menu_Segment_tex_set(menu_Segment* s, const int id, const menu_texC t) {
	if (s->tex == NULL)
		s->tex = calloc(s->dotsCnt, sizeof(menu_texC));
	menu_texC_cpy(s->tex, id, t);
}

void		menu_Segment_sprite_setProj(menu_Segment* s, uint16_t p_proj) {
	int	w	= menu_viewport.br[0]-menu_viewport.cur[0];
	int 	h	= menu_viewport.br[1]-menu_viewport.cur[1];
	int	sa	= 1;
	if (s->r->t->filter) sa = s->r->t->filter->scale;
	float	sw	= (float)w/((float)(s->r->t->orig_w));
	float	sh	= (float)h/((float)(s->r->t->orig_h));
	int	sc	= (int) sh;

	menu_texC  t0;
	t0[0] = (0.1)/s->r->t->w;
	t0[1] = (0.1)/s->r->t->h;			menu_Segment_tex_set(s, 0, t0);
	t0[0] = (s->r->t->orig_w*sa-0.1)/s->r->t->w;	menu_Segment_tex_set(s, 1, t0);
	t0[1] = (s->r->t->orig_h*sa-0.1)/s->r->t->h;	menu_Segment_tex_set(s, 2, t0);
	t0[0] = (0.1)/s->r->t->w;			menu_Segment_tex_set(s, 3, t0);


	switch(p_proj) {
	case 0:
		s->dots[9]	= s->dots[0]	= menu_viewport.cur[0];
		s->dots[4]	= s->dots[1]	= menu_viewport.cur[1];
		s->dots[6]	= s->dots[3]	= menu_viewport.br[0];
		s->dots[7]	= s->dots[10]	= menu_viewport.br[1];
	break;
	case 1:
		s->dots[9]	= s->dots[0]	= menu_viewport.cur[0];
		s->dots[4]	= s->dots[1]	= menu_viewport.cur[1];
		s->dots[6]	= s->dots[3]	= menu_viewport.br[0];
		s->dots[7]	= s->dots[10]	= menu_viewport.br[1];
		if (sw>sh) {
			s->dots[9]	= s->dots[0]	= menu_viewport.cur[0]+(((float)w-(float)s->r->t->orig_w*sh)/2);
			s->dots[6]	= s->dots[3]	= menu_viewport.br[0]-(((float)w-(float)s->r->t->orig_w*sh)/2);
		} else if (sw<sh) {
			s->dots[4]	= s->dots[1]	= menu_viewport.cur[1]+(((float)h-(float)s->r->t->orig_h*sw)/2);
			s->dots[7]	= s->dots[10]	= menu_viewport.br[1]-(((float)h-(float)s->r->t->orig_h*sw)/2);
		}
	break;
	case 2:
		if (sh>sw) sc = (int) sw;
		s->dots[9]	= s->dots[0]	= menu_viewport.cur[0]+((w-s->r->t->orig_w*sc)/2);
		s->dots[4]	= s->dots[1]	= menu_viewport.cur[1]+((h-s->r->t->orig_h*sc)/2);
		s->dots[6]	= s->dots[3]	= menu_viewport.br[0]-((w-s->r->t->orig_w*sc)/2);
		s->dots[7]	= s->dots[10]	= menu_viewport.br[1]-((h-s->r->t->orig_h*sc)/2);
	break;
	default:
	break;
	}
}

void		menu_Segment_set_sprite(menu_Segment* s, menu_vector p, const char *name) {
	menu_sprite_coord* sc		= menu_Texture_get_sprite(s->r->t, name);
	s->dots[9]	= s->dots[0]	= p[0];
	s->dots[4]	= s->dots[1]	= p[1];
	s->dots[6]	= s->dots[3]	= p[0]+sc->sw;
	s->dots[7]	= s->dots[10]	= p[1]+sc->sh;
	memcpy(s->tex, sc->tex, 8*sizeof(float));
}

void		menu_Segment_set_sprite_tex(menu_Segment* s, const char *name) {
	menu_sprite_coord* sc		= menu_Texture_get_sprite(s->r->t, name);
	memcpy(s->tex, sc->tex, 8*sizeof(float));
}

void		menu_Segment_move_by(menu_Segment* s, const menu_vector p) {
	int i, j;
	for(j=0;j<3;j++) {
		s->min[j] += p[j];
		s->max[j] += p[j];
	}
	for(i=0;i<s->dotsCnt;i++)
		for(j=0;j<3;j++) {
			s->dots[3*i+j] += p[j];
		}
}

void		menu_Segment_scale_by(menu_Segment* s, float b) {
	int i, j;
	float p[3];
	for(j=0;j<3;j++)
		p[j] = 0.0;
	for(i=0;i<s->dotsCnt;i++)
		for(j=0;j<3;j++)
			p[j] += s->dots[3*i+j];
	for(j=0;j<3;j++)
		p[j] = p[j]/s->dotsCnt;
	// now P is the iso-center of the object
	_menu_Segment_resetMm(s);

	for(i=0;i<s->dotsCnt;i++)
		for(j=0;j<3;j++) {
			s->dots[3*i+j] = p[j] - (p[j]-s->dots[3*i+j])*b;
			if(s->min[j]>s->dots[3*i+j]) s->min[j]=s->dots[3*i+j];
			if(s->max[j]<s->dots[3*i+j]) s->max[j]=s->dots[3*i+j];
		}
}

void		menu_Segment_rotate_by(menu_Segment* s, const menu_vector a) {
	int i, j;
	float p[3];
	float d[3];
	for(j=0;j<3;j++)
		p[j] = 0.0;
	for(i=0;i<s->dotsCnt;i++)
		for(j=0;j<3;j++)
			p[j] += s->dots[3*i+j];
	for(j=0;j<3;j++)
		p[j] = p[j]/s->dotsCnt;
	// now P is the iso-center of the object
	_menu_Segment_resetMm(s);

	for(i=0;i<s->dotsCnt;i++) {
		for(j=0;j<3;j++)
			d[j] = 0.0;
		for(j=0;j<3;j++) {
			// doing x rotation
			if (a[0]!=0.0){
			if (j==1) {
				d[1] += (s->dots[3*i+1]-p[1])*cosf(a[0]);
				d[1] -= (s->dots[3*i+2]-p[2])*sinf(a[0]);
			} else if (j==2) {
				d[2] -= (s->dots[3*i+1]-p[1])*sinf(a[0]);
				d[2] += (s->dots[3*i+2]-p[2])*cosf(a[0]);
			}}
			// doing y rotation
			if (a[1]!=0.0){
			if (j==0) {
				d[0] += (s->dots[3*i+0]-p[0])*cosf(a[1]);
				d[0] += (s->dots[3*i+2]-p[2])*sinf(a[1]);
			} else if (j==2) {
				d[2] -= (s->dots[3*i+0]-p[0])*sinf(a[1]);
				d[2] += (s->dots[3*i+2]-p[2])*cosf(a[1]);
			}}
			// doing z rotation
			if (a[2]!=0.0){
			if (j==0) {
				d[0] += (s->dots[3*i+0]-p[0])*cosf(a[2]);
				d[0] -= (s->dots[3*i+1]-p[1])*sinf(a[2]);
			} else if (j==1) {
				d[1] += (s->dots[3*i+0]-p[0])*sinf(a[2]);
				d[1] += (s->dots[3*i+1]-p[1])*cosf(a[2]);
			}}
		}
		for(j=0;j<3;j++) {
 			s->dots[3*i+j] = d[j] + p[j];
			if(s->min[j]>s->dots[3*i+j]) s->min[j]=s->dots[3*i+j];
			if(s->max[j]<s->dots[3*i+j]) s->max[j]=s->dots[3*i+j];
		}
	}
}

void		menu_Segment_rotate_sprite(menu_Segment* s, const float a) {
	int i, j;
	float p[3];
	float d[3];
	for(j=0;j<3;j++)
		p[j] = 0.0;
	for(i=0;i<s->dotsCnt;i++)
		for(j=0;j<3;j++)
			p[j] += s->dots[3*i+j];
	for(j=0;j<3;j++)
		p[j] = p[j]/s->dotsCnt;
	// now P is the iso-center of the object
	_menu_Segment_resetMm(s);

	for(i=0;i<s->dotsCnt;i++) {
		for(j=0;j<3;j++)
			d[j] = 0.0;
		for(j=0;j<3;j++) {
			// doing z rotation
			if (a!=0.0){
			if (j==0) {
				d[0] += (s->dots[3*i+0]-p[0])*cosf(a);
				d[0] -= (s->dots[3*i+1]-p[1])*sinf(a);
			} else if (j==1) {
				d[1] += (s->dots[3*i+0]-p[0])*sinf(a);
				d[1] += (s->dots[3*i+1]-p[1])*cosf(a);
			}}
		}
		for(j=0;j<3;j++) {
 			s->dots[3*i+j] = d[j] + p[j];
			if(s->min[j]>s->dots[3*i+j]) s->min[j]=s->dots[3*i+j];
			if(s->max[j]<s->dots[3*i+j]) s->max[j]=s->dots[3*i+j];
		}
	}
}

void		menu_Segment_sprite_hide(menu_Segment* s) {
	memset(s->dots, 0, s->dotsCnt*3*sizeof(float));
}
/***********************************************************************************
 *				render groups
 ***********************************************************************************/

menu_RGroup*	menu_RGroup_new(const uint16_t d, const GLenum m, const int cnt, const unsigned short* c, const Eina_Bool cols, const Eina_Bool tex) {
	menu_RGroup* ret		= calloc(1, sizeof(menu_RGroup));
	ret->mode		= m;
	ret->dotsCnt		= d;
	ret->dotsCur		= 0;
	ret->tilemap_w		= 0;
	ret->tilemap_h		= 0;
	ret->dots		= calloc(d*3, sizeof(float));
	ret->cols		= NULL;
	ret->tex		= NULL;
	ret->t			= NULL;
	ret->segs		= NULL;
	ret->shader		= NULL;
	ret->alpha		= EINA_FALSE;
	ret->tilehack		= EINA_FALSE;
	ret->doScissor		= EINA_FALSE;
	ret->indexLen		= cnt;
	ret->indexCur		= 0;
	ret->indexData		= calloc(cnt, sizeof(unsigned short));
	menu_color_set(ret->col, 1.0, 1.0, 1.0, 1.0);
	if (c!=NULL)	memcpy(ret->indexData, c, cnt*sizeof(unsigned short));
	//else		memset(ret->indexData, 0, cnt*sizeof(unsigned short));
	if (cols) ret->cols	= calloc(d, sizeof(menu_color));
	if (tex) ret->tex	= calloc(d, sizeof(menu_texC));
	memset(ret->scissor, 0, 4*sizeof(int32_t));
	return ret;
}

void		menu_RGroup_setScissor(menu_RGroup* m, int32_t left, int32_t bottom, int32_t width, int32_t height) {
	m->doScissor	= EINA_TRUE;
	m->scissor[0]	= left;
	m->scissor[1]	= menu_viewport.scrn[1]-bottom;
	m->scissor[2]	= width;
	m->scissor[3]	= height;
}

menu_RGroup*	menu_RGroupSprite_new(const int cnt, const Eina_Bool cols, const Eina_Bool tex) {
	return menu_RGroup_new(4*cnt, GL_TRIANGLE_STRIP, 9*cnt, NULL, cols, tex);
}


menu_RGroup*	menu_RGroup_from_Mesh(menu_Mesh* m) {
	if (m==NULL) return NULL;
	menu_RGroup* ret		= calloc(1, sizeof(menu_RGroup));
	ret->mode		= m->mode;
	ret->dotsCnt		= m->dots_count/3;
	ret->dotsCur		= m->dots_count/3;
	ret->dots		= NULL;
	ret->cols		= NULL;
	ret->tex		= NULL;
	ret->t			= NULL;
	ret->segs		= NULL;
	ret->alpha		= m->alpha;
	ret->tilehack		= m->is_tile;
	ret->doScissor		= EINA_FALSE;
	ret->tilemap_w		= m->tilemap_w;
	ret->tilemap_h		= m->tilemap_h;
	ret->indexLen		= m->faces_count;
	ret->indexCur		= m->faces_count;
	ret->indexData		= calloc(m->faces_count, sizeof(unsigned short));
	menu_color_copy(ret->col, m->col);
	memset(ret->scissor, 0, 4*sizeof(int32_t));
	memcpy(ret->indexData, m->faces, m->faces_count*sizeof(unsigned short));
	if(m->dots_count>0) {
		menu_RGroup_Texture_set(ret, menu_Theme_find_Texture(g, m->texture_name));
		ret->dots = calloc(m->dots_count, sizeof(float));
		memcpy(ret->dots, m->dots, m->dots_count*sizeof(float));
		if (m->cols_count>0) {
			ret->cols = calloc(m->cols_count, sizeof(float));
			memcpy(ret->cols, m->cols, m->cols_count*sizeof(float));
		}
		if (m->tex_count>0) {
			ret->tex = calloc(m->tex_count, sizeof(float));
			memcpy(ret->tex, m->tex, m->tex_count*sizeof(float));
		}
	}
	return ret;
}

void		menu_RGroup_free(menu_RGroup* m) {
	menu_Segment *s;

	if (m->dots != NULL)
		free(m->dots);
	if (m->cols != NULL)
		free(m->cols);
	if (m->tex != NULL)
		free(m->tex);
	if (m->indexData != NULL)
		free(m->indexData);

	EINA_LIST_FREE(m->segs, s) {
		menu_RGroup_Segment_free(m, s);
	}

	free(m);

}

void		menu_RGroup_empty(menu_RGroup* m) {
	menu_Segment *s;

	m->dotsCur		= 0;
	m->indexCur		= 0;
	EINA_LIST_FREE(m->segs, s) {
		menu_RGroup_Segment_free(m, s);
	}
}

void		menu_blend(Eina_Bool t) {
	if (t == use_blend)	return;
	use_blend = t;
	if (t) {
		GL_CHECK(glEnable(GL_BLEND));
	} else {
		GL_CHECK(glDisable(GL_BLEND));
	}
}
void		menu_RGroup_draw(menu_RGroup* m) {
	if (m->dots == NULL || m->dotsCur<1 || m->indexCur<1 )
		return;

	if (m->doScissor) {
		glScissor(m->scissor[0], m->scissor[1], m->scissor[2], m->scissor[3]);
		GL_CHECK(glEnable(GL_SCISSOR_TEST));
	}
	menu_blend(m->alpha);
		
#if defined(USE_SHADER)
	m->shader->draw(m->shader, m);
#else

	if (m->t != NULL && m->tex != NULL) {
		menu_Texture_useTex0(m->t);
		menu_Texture_bind(m->t);
		GL_CHECK(glTexCoordPointer(2, GL_FLOAT, 0, m->tex));
	} else menu_Texture_useTex0(NULL);

	if (m->cols != NULL) {
		glEnableClientState(GL_COLOR_ARRAY);
		GL_CHECK(glColorPointer(4, GL_FLOAT, 0, m->cols));
	} else
		glColor(m->col.t[0], m->col.t[1], m->col.t[2], m->col.t[3]);

	GL_CHECK(glVertexPointer(3, GL_FLOAT, 0, m->dots));
	if (m->tilehack) {
		int w = m->tilemap_h*9*41;
		int x = 9*((int)(menu_viewport.cur[0]/m->t->tile_w))*m->tilemap_h;
		if (w+x>m->indexCur)
			w=m->indexCur-x;
		GL_CHECK(glDrawElements(m->mode, w, GL_UNSIGNED_SHORT, m->indexData+x));
	} else {
		GL_CHECK(glDrawElements(m->mode, m->indexCur, GL_UNSIGNED_SHORT, m->indexData));
	}

	if (m->cols != NULL)
		GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
#endif

	if (m->doScissor)
		GL_CHECK(glDisable(GL_SCISSOR_TEST));
}

void		menu_RGroup_Texture_set(menu_RGroup* m, menu_Texture* t) {
	m->t  = t;
	if (t->format == menu_TEXTURE_FORMAT_4444 || t->format == menu_TEXTURE_FORMAT_ALPHA)
		m->alpha	= EINA_TRUE;
#if defined(USE_SHADER)
	if (m->shader==NULL)
		m->shader	= menu_Shader_texture;
#endif
}

menu_Segment*	menu_RGroup_Segment_new(menu_RGroup* m, const uint16_t d, const int cnt, const unsigned short* c) {
	int i;
	float *mem;
	unsigned short* memi;
	
	if (m->dotsCur+d>m->dotsCnt) {
		mem = realloc(m->dots, sizeof(menu_vector)*(m->dotsCnt+d*8));
		if (!mem) {
			printf("Error: realloc for dot from %d to %d: %s\n",m->dotsCnt,m->dotsCnt+d*8, strerror(errno));
			return NULL;
		} else if (m->dots != mem) {
			Eina_List*	l;
			menu_Segment*	s;
			EINA_LIST_FOREACH(m->segs, l, s) {
				s->dots += mem - m->dots; // set the new dots adress
			}
		}
		m->dots = mem;

		if (m->tex) {
			mem = realloc(m->tex, sizeof(menu_texC)*(m->dotsCnt+d*8));
			if (!mem) {
				printf("Error: realloc for tex from %d to %d: %s\n",m->dotsCnt,m->dotsCnt+d*8, strerror(errno));
				return NULL;
			}
			if (m->tex != mem) {
				Eina_List*	l;
				menu_Segment*	s;
				EINA_LIST_FOREACH(m->segs, l, s) {
					s->tex += mem - m->tex; // set the new tex adress
				}
			}
			m->tex = mem;
		}

		if (m->cols) {
			mem = realloc(m->cols, sizeof(menu_color)*(m->dotsCnt+d*8));
			if (!mem) {
				printf("Error: realloc for cols from %d to %d: %s\n",m->dotsCnt,m->dotsCnt+d*8, strerror(errno));
				return NULL;
			}
			if (m->cols != mem) {
				Eina_List*	l;
				menu_Segment*	s;
				EINA_LIST_FOREACH(m->segs, l, s) {
					s->cols += mem - m->cols; // set the new tex adress
				}
			}
			m->cols = mem;
		}

		m->dotsCnt += d*8;
	}
	if (cnt+m->indexCur+3>m->indexLen) {
		memi = realloc(m->indexData, sizeof(unsigned short)*(16*cnt+m->indexLen));
		if (!memi) {
			printf("Error: realloc for dot From %d to %d: %s\n",m->indexLen, 16*cnt+m->indexLen, strerror(errno));
			return NULL;
		}
		if (m->indexData != memi) {
			Eina_List*	l;
			menu_Segment*	s;
			EINA_LIST_FOREACH(m->segs, l, s) {
				s->indexData += memi - m->indexData; // set the new dots adress
			}
		}
		m->indexData = memi;
		m->indexLen += 16*cnt;
	}
	menu_Segment* ret	= menu_Segment_new(d, m);

	ret->dots		= m->dots+m->dotsCur*3;
	ret->indexLen		= cnt;
	ret->indexData		= m->indexData+sizeof(unsigned short)*(m->indexCur+1);
	if (m->tex) ret->tex	= m->tex+m->dotsCur*2;
	if (m->cols) ret->cols	= m->cols+m->dotsCur*4;

	if (c!=NULL) {
		if (m->indexCur>0) {
			m->indexData[m->indexCur+0] = m->indexData[m->indexCur-1];
			m->indexData[m->indexCur+1] = m->indexData[m->indexCur-1];
			m->indexData[m->indexCur+2] = c[0]+m->dotsCur;
			m->indexCur+=3;
		}
		for (i=0;i<cnt;i++)
			m->indexData[m->indexCur+i] = c[i]+m->dotsCur;
	} else
		memset(ret->indexData, 0, cnt);
	m->segs			= eina_list_append(m->segs, ret);
	m->dotsCur		+= d;
	m->indexCur		+= cnt;
	return ret;
}

void		menu_RGroup_Segment_free(menu_RGroup* m, menu_Segment* s) {
	m->segs			= eina_list_remove(m->segs, s);
	menu_Segment_free(s);
}

menu_Segment*	menu_RGroup_Mesh_add(menu_RGroup* rg, menu_Mesh* m) {
	int i,j;
	if (m==NULL || m->dots_count<1) return NULL;
	menu_Segment *s = menu_RGroup_Segment_new(rg, m->dots_count/3, m->faces_count, m->faces);
	menu_RGroup_Texture_set(rg, menu_Theme_find_Texture(g, m->texture_name));
	memcpy(s->dots, m->dots, m->dots_count*sizeof(float));
	if (m->cols_count>0)
		memcpy(s->cols, m->cols, m->cols_count*sizeof(float));
	if (m->tex_count>0)
		memcpy(s->tex, m->tex, m->tex_count*sizeof(float));

	_menu_Segment_resetMm(s);
	for(i=0;i<s->dotsCnt;i++) {
		for(j=0;j<3;j++) {
			if(s->min[j]>s->dots[3*i+j]) s->min[j]=s->dots[3*i+j];
			if(s->max[j]<s->dots[3*i+j]) s->max[j]=s->dots[3*i+j];
		}
	}
	return s;
}

menu_Segment*	menu_RGroup_Rect_add(menu_RGroup* m, menu_vector tl, menu_vector br) {
	menu_vector p0;
	menu_Segment *s = menu_RGroup_Segment_new(m, 4, 6, squareIndex);
	p0[0] = tl[0];		p0[2] = tl[2];
	p0[1] = tl[1];		menu_Segment_dot_set(s, 0, p0);
	p0[0] = br[0];		menu_Segment_dot_set(s, 1, p0);
	p0[1] = br[1];		menu_Segment_dot_set(s, 2, p0);
	p0[0] = tl[0];		menu_Segment_dot_set(s, 3, p0);
	return s;
}

menu_Segment*	menu_RGroup_Tile_add(menu_RGroup* m, menu_vector p, int32_t id) {
	int maxts, ix, iy;
	menu_vector tl = {0.0, 0.0, 0.0};
	menu_vector br = {0.0, 0.0, 0.0};

	if (! m->t->tilesheet) return NULL;
	if (id == 0) return NULL;
	maxts = m->t->orig_w/m->t->tile_w;
	iy = (id-1)/maxts;
	ix = (id-1) - iy*maxts;
	tl[0]= ix*m->t->tile_w;
	tl[1]= iy*m->t->tile_h;
	br[0]= (ix+1)*m->t->tile_w;
	br[1]= (iy+1)*m->t->tile_h;

	return menu_RGroup_TexturePart_add(m, p, tl, br);
}

menu_Segment*	menu_RGroup_Sprite_add(menu_RGroup* m, menu_vector p, const char *name) {
	menu_Segment *s;
	menu_vector br = {0.0, 0.0, 0.0};
	menu_sprite_coord* sc = menu_Texture_get_sprite(m->t, name);
	if (sc==NULL) return NULL;
	br[0] = p[0] + sc->sw;
	br[1] = p[1] + sc->sh;
	s = menu_RGroup_Rect_add(m, p, br);
	memcpy(s->tex, sc->tex, 8*sizeof(float));
	return s;
}

menu_Segment*	menu_RGroup_Image_add(menu_RGroup* m, int tlx, int tly, int brx, int bry) {
	menu_Segment *s;
	menu_texC  t0;
	menu_vector tl = {(float)tlx, (float)tly, 0.05};
	menu_vector br = {(float)brx, (float)bry, 0.05};
	s = menu_RGroup_Rect_add(m, tl, br);
	t0[0] = (0.1)/m->t->w;
	t0[1] = (0.1)/m->t->h;			menu_Segment_tex_set(s, 0, t0);
	t0[0] = (m->t->orig_w-0.1)/m->t->w;	menu_Segment_tex_set(s, 1, t0);
	t0[1] = (m->t->orig_h-0.1)/m->t->h;	menu_Segment_tex_set(s, 2, t0);
	t0[0] = (0.1)/m->t->w;			menu_Segment_tex_set(s, 3, t0);
	return s;
}


menu_Segment*	menu_RGroup_TexturePart_add(menu_RGroup* m, menu_vector p, menu_vector tl, menu_vector br) {
	menu_texC  t0;
	menu_vector p0;
	menu_Segment *s = menu_RGroup_Segment_new(m, 4, 6, squareIndex);
	t0[0] = (tl[0]+0.1)/m->t->w;
	t0[1] = (tl[1]+0.1)/m->t->h;
	p0[0] = p[0];
	p0[1] = p[1];
	p0[2] = p[2];
	menu_Segment_dot_set(s, 0, p0);	menu_Segment_tex_set(s, 0, t0);
	t0[0] = (br[0]-0.1)/m->t->w;
	p0[0] = p[0] + br[0] - tl[0];
	menu_Segment_dot_set(s, 1, p0);	menu_Segment_tex_set(s, 1, t0);
	t0[1] = (br[1]-0.1)/m->t->h;
	p0[1] = p[1] + br[1] - tl[1];
	menu_Segment_dot_set(s, 2, p0);	menu_Segment_tex_set(s, 2, t0);
	t0[0] = (tl[0]+0.1)/m->t->w;
	p0[0] = p[0];
	menu_Segment_dot_set(s, 3, p0);	menu_Segment_tex_set(s, 3, t0);
	return s;
}

menu_Segment*	menu_RGroup_Tilemap_Tile_add(menu_RGroup* m, int32_t x, int32_t y, int32_t id)  {
	int maxts, ix, iy;
	menu_vector p  = {0.0, 0.0, 0.0};
	menu_vector tl = {0.0, 0.0, 0.0};
	menu_vector br = {0.0, 0.0, 0.0};

	if (! m->t->tilesheet) return NULL;
	if (id == 0) return NULL;
	maxts = m->t->orig_w/m->t->tile_w;
	iy = (id-1)/maxts;
	ix = (id-1) - iy*maxts;
	p[0] = x*m->t->tile_w;
	p[1] = y*m->t->tile_h;
	tl[0]= ix*m->t->tile_w;
	tl[1]= iy*m->t->tile_h;
	br[0]= (ix+1)*m->t->tile_w;
	br[1]= (iy+1)*m->t->tile_h;

	return menu_RGroup_TexturePart_add(m, p, tl, br);
}


void		menu_RGroup_set_font(menu_RGroup* m, menu_Font *f) {
	if (f==NULL || f->texture == NULL)	return;
	m->f = f;
#if defined(USE_SHADER)
	if (m->shader==NULL)
		m->shader	= menu_Shader_font;
#endif
	if(menu_viewport.scaleFactor>1)
		menu_RGroup_Texture_set(m,f->texture2);
	else
		menu_RGroup_Texture_set(m,f->texture);
	
}

static char **	menu_split_text(const char* p_txt) {
	char **res;
	int count = 0, i = 0;
	char *txt = (char*)p_txt;
	if (txt == NULL) return NULL;
	while (*txt) {
		if (*txt == ' ')
			count++;
		txt++;
	}
	res = calloc(count+2, sizeof(char *)); //1 mot de plus que d'espace + 1 NULL a la fin
	res[0] = calloc(strlen(p_txt)+1, sizeof(char));
	strcpy(res[0], p_txt);
	txt = res[0];
	while (*txt) {
		if (*txt == ' ') {
			*txt=0;
			res[++i] = txt+1;
		}
		txt++;
	}
	return res;
	
}
static float menu_word_len(menu_RGroup* m, char *text) {
	float 			x = 0;
	float 			y = 50;
	stbtt_bakedchar*	cdata;
	stbtt_aligned_quad	q;

	if (!m || !m->f|| !text) return 0.0;

	if(menu_viewport.scaleFactor>1)
		cdata = m->f->cdata2;
	else
		cdata = m->f->cdata;
	if (cdata==NULL) return  0.0;
	while (*text) {
		if (*text >= 32 && *text < 128) {
			stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);
		}
		++text;
	}
	return x;
}
void		menu_RGroup_Text_add_bounded(menu_RGroup* m, const char* text, menu_vector topleft, menu_vector bottomright) {
	float 			x = topleft[0], x1;
	float 			y = topleft[1];
	float 			w = bottomright[0] - topleft[0];
	float			sp, len = 0;
	menu_vector		p;
	int 			i, j, t, n = 0;
	stbtt_bakedchar*	cdata;
	stbtt_aligned_quad	q;
	char**			at = menu_split_text(text);

	menu_vector_set(p, 0, topleft[0], topleft[1], topleft[2]);
	if (!m || !m->f || !at) return;

	if(menu_viewport.scaleFactor>1)
		cdata = m->f->cdata2;
	else
		cdata = m->f->cdata;
	if (cdata==NULL) return;
	stbtt_GetBakedQuad(cdata, 512,512, ' '-32, &x,&y,&q,1);
	x -= topleft[0]; x1=topleft[0];
	stbtt_GetBakedQuad(cdata, 512,512, 'I'-32, &x1,&y,&q,1);
	y  = q.y1-q.y0;

	while(at[n] && p[1]<bottomright[1]) {
		// find the last word for the line
		len = 0;
		sp = x;
		for(i=n; at[i] && len <w;i++){
			t = menu_word_len(m,at[i]);
			if (len+t+sp>w && len>0)
				break;
			len += t+sp;
		}
		len-= sp;
		// find the spacing for each words
		j=i-n-1;
		if (j > 0 && at[i])
			sp = (w-len+j*sp)/j;
		if (sp < x)
			sp = x;
		// output
		j=n;
		do {
			menu_RGroup_Text_add_at(m, at[j], p);
			p[0] += menu_word_len(m,at[j]) + sp;
			j++;
		} while (j<i);
		p[0] = topleft[0];
		p[1] += round(y*1.5);
		n=i;
	}

	free(at[0]);
	free(at);
}


void		menu_RGroup_Text_add_at(menu_RGroup* m, const char* text, menu_vector p) {
	float 			x = p[0];
	float 			y = p[1];
	stbtt_bakedchar*	cdata;
	stbtt_aligned_quad	q;

	if (!m || !m->f) return;

	if(menu_viewport.scaleFactor>1)
		cdata = m->f->cdata2;
	else
		cdata = m->f->cdata;
	if (cdata==NULL) return;
	stbtt_GetBakedQuad(cdata, 512,512, 1, &x,&y,&q,1);
	x = p[0];
	y+= q.y1-q.y0;
	menu_texC  t0;
	menu_vector p0;

	while (*text) {
		if (*text >= 32 && *text < 128) {
			stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);
			menu_Segment *s = menu_RGroup_Segment_new(m, 4, 6, squareIndex);
			if (s==NULL) return;
			t0[0] = q.s0;
			t0[1] = q.t0;
			p0[0] = q.x0;
			p0[1] = q.y0;
			p0[2] = p[2];
			menu_Segment_dot_set(s, 0, p0);	menu_Segment_tex_set(s, 0, t0);
			t0[0] = q.s1;
			p0[0] = q.x1;
			menu_Segment_dot_set(s, 1, p0);	menu_Segment_tex_set(s, 1, t0);
			t0[1] = q.t1;
			p0[1] = q.y1;
			menu_Segment_dot_set(s, 2, p0);	menu_Segment_tex_set(s, 2, t0);
			t0[0] = q.s0;
			p0[0] = q.x0;
			menu_Segment_dot_set(s, 3, p0);	menu_Segment_tex_set(s, 3, t0);
		}
		++text;
	}
}
void		menu_RGroup_Text_add(menu_RGroup* m, menu_Text *t) {
	menu_RGroup_Text_add_at(m,t->text, t->pos);
}

void		menu_RGroup_uniq_col_set(menu_RGroup* s, const menu_color c) {
	menu_color_copy(s->col, c);
#if defined(USE_SHADER)
	if (s->shader==NULL)
		s->shader	= menu_Shader_color;
#endif

}

void		menu_RGroup_move_by(menu_RGroup* rg, const menu_vector p) {
	menu_Segment *s;
	Eina_List*	l;
	EINA_LIST_FOREACH(rg->segs, l, s) {
		menu_Segment_move_by(s, p);
	}
}


/***********************************************************************************
 *				Textures
 ***********************************************************************************/


static menu_Texture *lastBindedTex = NULL;
#define B sp[i - b]
#define D sp[i - (i>0?1:0)]
#define F sp[i + (i<w?1:0)]
#define H sp[i + h0]
#define E  sp[i]
#define E0 tp[i*2] 
#define E1 tp[i*2 + 1]
#define E2 tp[i*2 + tpitch]
#define E3 tp[i*2 + 1 + tpitch]

static uint16_t* scale2x(uint16_t* src, const int w, const int h) {
	register int i, j;
	int b, h0;

	uint16_t* ret = calloc(2*w*2*h, sizeof(uint16_t));

	int tpitch = 2*w;
	int spitch = w;
	uint16_t* tp = ret;
	uint16_t* sp = src;
 
	for (j = 0; j < h; ++j) {
		b = j>0?spitch:0;
		h0 = j<h?spitch:0;
		for (i = 0; i < w; ++i) {
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = B == F ? F : E;
				E2 = D == H ? D : E;
				E3 = H == F ? F : E;
			} else {
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
			}
		}
		tp += 2*tpitch;
		sp += spitch;
	}
	return ret;
}
static unsigned char* scale2xa(unsigned char* src, const int w, const int h) {
	register int i, j;
	int b, h0;

	unsigned char* ret = calloc(2*w*2*h, sizeof(unsigned char));

	int tpitch = 2*w;
	int spitch = w;
	unsigned char* tp = ret;
	unsigned char* sp = src;
 
	for (j = 0; j < h; ++j) {
		b = j>0?spitch:0;
		h0 = j<h?spitch:0;
		for (i = 0; i < w; ++i) {
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = B == F ? F : E;
				E2 = D == H ? D : E;
				E3 = H == F ? F : E;
			} else {
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
			}
		}
		tp += 2*tpitch;
		sp += spitch;
	}
	return ret;
}

#undef B
#undef D
#undef F
#undef H
#undef E
#undef E0
#undef E1
#undef E2
#undef E3

void		menu_Texture_useTex0(menu_Texture *t) {
	Eina_Bool p = EINA_FALSE;
#ifdef HAVE_GLES
	if (use_texPBO && t==NULL) {
		glDisable(GL_TEXTURE_STREAM_IMG);
		use_texPBO = EINA_FALSE;
	}else if (!use_texPBO && t!=NULL && t->use_PBO) {
		glEnable(GL_TEXTURE_STREAM_IMG);
		use_texPBO = EINA_TRUE;
	}
	p = EINA_TRUE;
#endif

	if (use_tex0 && t==NULL) {
#if !defined(USE_SHADER)
		GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		GL_CHECK(glDisable(GL_TEXTURE_2D));
#endif
		use_tex0 = EINA_FALSE;
	} else if (!use_tex0 && t!=NULL && (!t->use_PBO || !p)) {
#if defined(USE_SHADER)
		GL_CHECK(glActiveTexture(GL_TEXTURE0));
#else
		GL_CHECK(glEnable(GL_TEXTURE_2D));
		GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
#endif
		use_tex0 = EINA_TRUE;
	}
}


void		menu_Texture_bind(menu_Texture *t) {
	if (t==lastBindedTex) return;
#ifdef HAVE_GLES
	if (t->use_PBO) {
		int i=0;
		//if (t->data == t->stream2) i=1;
		glBindTexture (GL_TEXTURE_STREAM_IMG, t->id);
		glTexBindStreamIMG(t->pbo, i);
	} else
#endif	
		glBindTexture(GL_TEXTURE_2D, t->id);

	lastBindedTex = t;
}

menu_Texture*	menu_Texture_newUpdatable(const char *p_name, const unsigned int w, const unsigned int h) {
	GLuint i;
	menu_Texture* ret = menu_Texture_new(p_name, w, h);
/*#ifndef HAVE_GLES
	if (menu_havePBO) {
		ret->use_PBO = EINA_TRUE;
		glGenBuffers (1, &(ret->pbo));
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, ret->pbo);
		glBufferData (GL_PIXEL_UNPACK_BUFFER,
				ret->w * ret->h * 2,
				NULL, GL_STREAM_DRAW);
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
	}
#endif*/
	GL_CHECK(glGenTextures(1, &i));
	ret->id		= i;
	ret->format	= menu_TEXTURE_FORMAT_656;
	menu_Texture_bind(ret);
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ret->w, ret->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL));
	menu_Texture_setAntiAlias(ret, EINA_FALSE);
	return ret;
}

menu_Texture*	menu_Texture_newStream(const char *p_name, const unsigned int w, const unsigned int h, const int maxw) {
	GLuint i;
	Eina_Bool p = EINA_TRUE;
	menu_Texture* ret = menu_Texture_new(p_name, maxw*4, maxw*4);
	if (menu_havePBO) {
		ret->use_PBO = EINA_TRUE;
#ifndef HAVE_GLES
		glGenBuffers (1, &(ret->pbo));
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, ret->pbo);
		glBufferData (GL_PIXEL_UNPACK_BUFFER,
				ret->w * ret->h * 2,
				NULL, GL_STREAM_DRAW);
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
#else
		p		= EINA_FALSE;
		ret->pbo	= pboID++;
		menu_alloc_buff(ret->pbo, maxw*4, maxw*4);
		free(ret->data);
		ret->stream1 = (uint16_t*)buf_vaddr[ret->pbo];
		//ret->stream2 = (uint16_t*)buf_vaddr2[ret->pbo];
		ret->data = ret->stream1;
#endif
	}
	GL_CHECK(glGenTextures(1, &i));
	ret->id		= i;
	ret->orig_h	= h;
	ret->orig_w	= w;
	ret->format	= menu_TEXTURE_FORMAT_656;
	ret->isStream	= EINA_TRUE;
	ret->sourceData	= calloc(maxw*maxw, sizeof(uint16_t));
	ret->filter	= menu_filter_get(0);
	menu_Texture_bind(ret);
	if (p) {
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ret->w, ret->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL));
	}
	menu_Texture_setAntiAlias(ret, EINA_FALSE);
	//menu_Theme_add_Texture(g, ret);
	return ret;
}

menu_Texture*    menu_Texture_newAlpha(const char *p_name, const unsigned int w, const unsigned int h, unsigned char *bmp) {
	GLuint i;
	menu_Texture* ret = menu_Texture_new(p_name, w, h);
	GL_CHECK(glGenTextures(1, &i));
	ret->id		= i;
	ret->format	= menu_TEXTURE_FORMAT_ALPHA;
	menu_Texture_bind(ret);
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmp));
	// can free temp_bitmap at this point
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	menu_Theme_add_Texture(g, ret);
	return ret;
}

void		menu_Texture_setAntiAlias(menu_Texture *t, Eina_Bool a) {
	GLint	f = GL_NEAREST;
	if (a)	f = GL_LINEAR;
	menu_Texture_bind(t);
#ifdef HAVE_GLES
	if (t->use_PBO) {
		glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MIN_FILTER, f);
		glTexParameterf(GL_TEXTURE_STREAM_IMG, GL_TEXTURE_MAG_FILTER, f);
	} 
#endif
	{
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, f));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, f));
	}
}

void		menu_Texture_update(menu_Texture *t) {
#ifndef HAVE_GLES
	void  *pboMemory = NULL;
	int    y;
#endif
	int sc = 1;

	menu_Texture_bind(t);
	if (t->filter && t->isUpdated) {
#ifdef USE_RENDER_THREAD
		SDL_LockMutex(rendering_mutex);
#endif
		t->filter->proc(t->sourceData, t->data, t->orig_w, t->w/2, t->w*2, t->orig_h);
		t->isUpdated = EINA_FALSE;
		sc = t->filter->scale;
#ifdef USE_RENDER_THREAD
		SDL_UnlockMutex(rendering_mutex);
#endif
	}else if (!t->isUpdated)
		return;
	else
		t->isUpdated = EINA_FALSE;

	if (t->use_PBO) {
#ifndef HAVE_GLES
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, t->pbo);
		pboMemory = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (!pboMemory) return;
		for (y = 0; y < t->orig_h * sc; y++) {
			memcpy (pboMemory + (t->w * y*2),
                        t->data + (y * t->w),
                        t->orig_w*2 * sc);
		}
		glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);
		GL_CHECK(glPixelStorei (GL_UNPACK_ROW_LENGTH, t->w));
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0, t->orig_w * sc, t->orig_h * sc, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL));
		GL_CHECK(glPixelStorei (GL_UNPACK_ROW_LENGTH, 0));
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
#endif
	} else if (t->format == menu_TEXTURE_FORMAT_656) {
#ifdef GL_UNPACK_ROW_LENGTH
		GL_CHECK(glPixelStorei (GL_UNPACK_ROW_LENGTH, t->w));
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0, t->orig_w * sc, t->orig_h * sc, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, t->data));
		GL_CHECK(glPixelStorei (GL_UNPACK_ROW_LENGTH, 0));
#else
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0, t->w, t->orig_h * sc, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, t->data));
		/* too slow to be any usable
		for(y=0;y<t->orig_h;y++) {
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,y, t->orig_w, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, t->data+y*t->w));
		}*/
#endif
	}
}

void		menu_Texture_load(menu_Texture *t) {
	GLuint i;
	int scale = 1;
	if (t==NULL) return;
	void *buf = t->data;

	if (menu_viewport.scaleFactor==2 || t->scale) {
		scale = 2;
		if (t->format!=menu_TEXTURE_FORMAT_ALPHA)
			buf = scale2x(t->data, t->w, t->h);
		else
			buf = scale2xa((unsigned char *)t->data, t->w, t->h);
	}
	glGenTextures(1, &i);
	t->id = i;
	menu_Texture_bind(t);
	if (t->format == menu_TEXTURE_FORMAT_656) {
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w*scale, t->h*scale, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL));
		menu_Texture_setAntiAlias(t, EINA_FALSE);
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0, t->orig_w*scale, t->orig_h*scale, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buf));
	} else if (t->format == menu_TEXTURE_FORMAT_4444) {
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w*scale, t->h*scale, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, NULL));
		menu_Texture_setAntiAlias(t, EINA_FALSE);
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,0,0,0, t->orig_w*scale, t->orig_h*scale, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, buf));
	} else {
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, t->w*scale, t->h*scale, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	}
	if (menu_viewport.scaleFactor==2 || t->scale)
		free(buf);
}

void		menu_Texture_unload(menu_Texture *t) {
	GL_CHECK(glDeleteTextures(1, &(t->id)));
}

void		menu_Texture_Theme_init(menu_Themes *g) {
	Eina_List*	l;
	menu_Texture*	t;
	menu_Font*	f;
	EINA_LIST_FOREACH(g->textures, l, t) {
		menu_Texture_load(t);
	}
	EINA_LIST_FOREACH(g->fonts, l, f) {
		f->texture  = menu_Theme_find_Texture(g, f->tex_name);
		f->texture2 = menu_Theme_find_Texture(g, f->tex_name2);
	}
}

void		menu_Texture_Theme_shutdown(menu_Themes *g) {
	Eina_List*	l;
	menu_Texture*	t;
	EINA_LIST_FOREACH(g->textures, l, t) {
		menu_Texture_unload(t);
	}
}
