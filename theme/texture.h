
#ifndef __menu_TEXTURE_H__
#define __menu_TEXTURE_H__

#include <Eina.h>
#include <stdint.h>
#include "stb_truetype.h"
#ifdef HAVE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __menu_FILTERS_H__
typedef struct menu_Filter_ menu_Filter;
#endif

#define menu_TEXTURE_FORMAT_656   0
#define menu_TEXTURE_FORMAT_4444  1
#define menu_TEXTURE_FORMAT_ALPHA 2
typedef float		menu_texC[2];
typedef float		menu_vector[3];
typedef float		menu_matrix[4][4];
typedef float		menu_quater[4];
typedef union {
	struct {
		float r;
		float g;
		float b;
		float a;
	} s;

	menu_quater t;
} menu_color;

typedef struct menu_sprite_coord_ menu_sprite_coord;
struct menu_sprite_coord_ {
	const char*	name;
	float		tex[8]; // 4 dots, 2 coords/dot
	uint16_t	sw, sh;
};

typedef struct menu_Texture_ menu_Texture;
struct menu_Texture_ {
	const char*	name;
 	uint16_t	w, h;
 	uint16_t	orig_w, orig_h;
	uint16_t	tile_w, tile_h;
	uint16_t*	data;
	Eina_List*	sprites;
	GLuint		id;
	uint8_t		format;
	Eina_Bool	spritesheet;
	Eina_Bool	tilesheet;
	Eina_Bool	scale;

	Eina_Bool	isStream;
	Eina_Bool	isUpdated;
	Eina_Bool	use_PBO;
	uint16_t*	stream1;
	uint16_t*	stream2;
	GLuint		pbo;
	uint16_t*	sourceData;
	menu_Filter*	filter;
};


#define menu_texC_set(a, i, x, y) {a[2*i]=x;a[2*i+1]=y;}
#define menu_texC_cpy(a, i, s) {memcpy(&a[2*i], s, sizeof(menu_texC));}
#define menu_texC_cpys(a, i, s, c) {memcpy(&a[2*i], s, c*sizeof(menu_texC));}
#define menu_vector_set(a, i, x, y, z) {a[3*i]=x;a[3*i+1]=y;a[3*i+2]=z;}
#define menu_vector_cpy(a, i, s) {memcpy(&a[3*i], &s[0], sizeof(menu_vector));}
#define menu_color_set(v, xr, xg, xb, xa) {v.s.r=xr;v.s.g=xg;v.s.b=xb;v.s.a=xa;}
#define menu_color_cpy(v, i, s) {memcpy(&v[4*i], &s, sizeof(menu_color));}
#define menu_color_cpys(v, i, s,c) {memcpy(&v[4*i], &s, c*sizeof(menu_color));}
#define menu_color_copy(v, s) {memcpy(&v, &s, sizeof(menu_color));}

void		menu_Texture_free(menu_Texture *t);
menu_Texture*	menu_Texture_new(const char *p_name, const unsigned int w, const unsigned int h);
menu_Texture*	menu_Texture_load_from(const char *p_name, const char *p_filename);
void		menu_Texture_add_sprite(menu_Texture* t, const char *s_name, menu_texC tl, menu_texC br);
menu_sprite_coord* menu_Texture_get_sprite(menu_Texture* t, const char *s_name);

typedef struct {
	const char*	name;
	char*		tex_name;
	menu_Texture *	texture;
	stbtt_bakedchar	cdata[96];
	char*		tex_name2;
	menu_Texture *	texture2;
	stbtt_bakedchar	cdata2[96];
} menu_Font;

menu_Font*	menu_Font_new(const char *ttfFile, const unsigned int size, const char *name);
void		menu_Font_free(menu_Font* f);

#if defined(__cplusplus)
}
#endif
#endif
