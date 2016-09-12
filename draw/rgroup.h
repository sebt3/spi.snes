#ifndef __menu_RGROUP_H__
#define __menu_RGROUP_H__
#include "theme.h"
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct menu_Shader_ menu_Shader;
typedef struct menu_RGroup_ menu_RGroup;

typedef struct {
	menu_RGroup*	r;
	float*		dots;
	float*		cols;
	float*		tex;
	uint16_t	dotsCnt;

	unsigned short*	indexData;
	uint16_t	indexLen;

	menu_vector	min, max;
} menu_Segment;

typedef struct menu_Shader_ menu_Shader;

struct menu_RGroup_ {
	GLenum		mode;
	Eina_Bool	alpha;
	Eina_Bool	tilehack;
	uint16_t	tilemap_w;
	uint16_t	tilemap_h;

	Eina_Bool	doScissor;
	int32_t		scissor[4];

	float*		dots;
	float*		cols;
	menu_color	col;
	float*	tex;
	uint32_t	dotsCur;
	uint32_t	dotsCnt;

	unsigned short*	indexData;
	uint16_t	indexCur;
	uint16_t	indexLen;

	menu_Texture*	t;
	menu_Font*	f;

	Eina_List*	segs;
	menu_Shader*	shader;
};


typedef void (*menu_shader_draw)(menu_Shader* s, menu_RGroup* r);

struct menu_Shader_ {
	const char*		name;
	GLuint			vertx;
	GLuint			fragm;
	GLuint			prgm;
	GLuint			a_position;
	GLuint			a_color;
	GLuint			a_texCoord0;
	GLuint			u_projection;
	GLuint			u_color;
	GLuint			u_param;
	menu_shader_draw	draw;
};

/***********************************************************************************
 *				Shaders
 ***********************************************************************************/
#ifndef HAVE_GLES
#define SHADER_MAX_ID 9
#else
#define SHADER_MAX_ID 3
#endif
void		menu_Shader_init();
menu_Shader*	menu_Shader_get_filter(uint16_t id);

/***********************************************************************************
 *				Runtime Texture
 ***********************************************************************************/
void		menu_Texture_bind(menu_Texture *t);
void		menu_Texture_load(menu_Texture *t);
void		menu_Texture_update(menu_Texture *t);
void		menu_Texture_unload(menu_Texture *t);
void		menu_Texture_Theme_init(menu_Themes *g);
void		menu_Texture_Theme_shutdown(menu_Themes *g);
void		menu_Texture_setAntiAlias(menu_Texture *t, Eina_Bool a);
menu_Texture*	menu_Texture_newUpdatable(const char *p_name, const unsigned int w, const unsigned int h);
menu_Texture*	menu_Texture_newStream(const char *p_name, const unsigned int w, const unsigned int h, const int maxw);
menu_Texture*   menu_Texture_newAlpha(const char *p_name, const unsigned int w, const unsigned int h, unsigned char *bmp);

/***********************************************************************************
 *				Setup segments
 ***********************************************************************************/

void		menu_Segment_col_set(menu_Segment* s, const int id, const menu_color c);
void		menu_Segment_tex_set(menu_Segment* s, const int id, const menu_texC t);
void		menu_Segment_dot_set(menu_Segment* s, const int id, const menu_vector p);

/***********************************************************************************
 *				runtime segments functions
 ***********************************************************************************/

void		menu_Segment_move_by(menu_Segment* s, const menu_vector p);
void		menu_Segment_scale_by(menu_Segment* s, float b);
void		menu_Segment_rotate_by(menu_Segment* s, const menu_vector a);
void		menu_Segment_rotate_sprite(menu_Segment* s, const float a);
void		menu_Segment_set_sprite(menu_Segment* s, menu_vector p, const char *name);
void		menu_Segment_set_sprite_tex(menu_Segment* s, const char *name);
void		menu_Segment_sprite_setProj(menu_Segment* s, uint16_t p_proj);
void		menu_Segment_sprite_hide(menu_Segment* s);

/***********************************************************************************
 *				Setup render groups
 ***********************************************************************************/

menu_RGroup*	menu_RGroup_new(const uint16_t d, const GLenum m, const int cnt, const unsigned short* c, const Eina_Bool cols, const Eina_Bool tex);
menu_RGroup*	menu_RGroupSprite_new(const int cnt, const Eina_Bool cols, const Eina_Bool tex);
menu_RGroup*	menu_RGroup_from_Mesh(menu_Mesh* m);
void		menu_RGroup_free(menu_RGroup* m);

void		menu_RGroup_set_font(menu_RGroup* m, menu_Font *f);
void		menu_RGroup_Texture_set(menu_RGroup* m, menu_Texture* t);
void		menu_RGroup_uniq_col_set(menu_RGroup* s, const menu_color c);

/***********************************************************************************
 *				Adding objects
 ***********************************************************************************/

void		menu_RGroup_Text_add(menu_RGroup* m, menu_Text *t);
void		menu_RGroup_Text_add_at(menu_RGroup* m, const char* text, menu_vector p);
void		menu_RGroup_Text_add_bounded(menu_RGroup* m, const char* text, menu_vector topleft, menu_vector bottomright);
menu_Segment*	menu_RGroup_Rect_add(menu_RGroup* m, menu_vector tl, menu_vector br);
menu_Segment*	menu_RGroup_Image_add(menu_RGroup* m, int tlx, int tly, int brx, int bry);
menu_Segment*	menu_RGroup_TexturePart_add(menu_RGroup* m, menu_vector p, menu_vector tl, menu_vector br);
menu_Segment*	menu_RGroup_Tilemap_Tile_add(menu_RGroup* m, int32_t x, int32_t y, int32_t id);
menu_Segment*	menu_RGroup_Tile_add(menu_RGroup* m, menu_vector p, int32_t id);
menu_Segment*	menu_RGroup_Sprite_add(menu_RGroup* m, menu_vector p, const char *name);
menu_Segment*	menu_RGroup_Mesh_add(menu_RGroup* rg, menu_Mesh* m);

/***********************************************************************************
 *				runtime render groups functions
 ***********************************************************************************/
void		menu_RGroup_empty(menu_RGroup* m);
void		menu_RGroup_draw(menu_RGroup* m);
void		menu_RGroup_move_by(menu_RGroup* rg, const menu_vector p);
void		menu_RGroup_setScissor(menu_RGroup* m, int32_t left, int32_t bottom, int32_t width, int32_t height);

#if defined(__cplusplus)
}
#endif
#endif
