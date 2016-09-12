#ifndef __menu_MESH_H__
#define __menu_MESH_H__
#include "texture.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	const char*	name;
	GLenum		mode;
	Eina_Bool	alpha;
	Eina_Bool	use_sub;
	Eina_Bool	is_tile;
	Eina_Bool	loaded;
	uint16_t	tilemap_w;
	uint16_t	tilemap_h;
	

	// Materia data
	menu_color	col;	// physical color (used when not using "cols")
	menu_color	ambiant;// Ka
	menu_color	difuse;	// Kd
	menu_color	specular; // Ks
	menu_color	transfilter; // Tf
	uint16_t	illum;	// illumination mode

	// standard version
	float*		dots;
	int32_t		dots_count;
	uint32_t	dots_max;
	float*		cols;
	int32_t		cols_count;
	float*		tex;
	int32_t		tex_count;
	float*		norm;
	int32_t		norm_count;

	unsigned short*	faces;
	int32_t		faces_count;
	uint32_t	faces_max;

	menu_Texture*	t;
	const char*	texture_name;
} menu_Mesh;

typedef struct {
	const char*	name;
	const char*	font;
	      char*	text;
	menu_vector	pos;
} menu_Text;

menu_Text*	menu_Text_new(const char *m_name, char *font, float* p_pos, char *p_text);
void		menu_Text_free(menu_Text* s);

menu_Mesh*	menu_Mesh_new(const char *m_name, const uint32_t d, const uint32_t f, const GLenum m);
void		menu_Mesh_free(menu_Mesh* s);
void		menu_Mesh_finalize(menu_Mesh* s);
void		menu_Mesh_finish_sub_mesh(menu_Mesh* s, const uint16_t d);
void		menu_Mesh_Texture_set(menu_Mesh* s, menu_Texture* t);
void		menu_Mesh_uniq_col_set(menu_Mesh* s, const menu_color c);

// sub-mesh management
void		menu_Mesh_finish_sub(menu_Mesh* s, const uint16_t d, const uint16_t f, const unsigned short* idx);
void		menu_Mesh_col_sub(menu_Mesh* s, const int id, const menu_color c);
void		menu_Mesh_tex_sub(menu_Mesh* s, const int id, const menu_texC t);
void		menu_Mesh_dot_sub(menu_Mesh* s, const int id, const menu_vector p);
void		menu_Mesh_norm_sub(menu_Mesh* s, const int id, const menu_vector p);

// if no sub mesh are used (but remember to set the indexes using menu_Mesh_finish_sub
void		menu_Mesh_col_set(menu_Mesh* s, const int id, const menu_color c);
void		menu_Mesh_tex_set(menu_Mesh* s, const int id, const menu_texC t);
void		menu_Mesh_dot_set(menu_Mesh* s, const int id, const menu_vector p);
void		menu_Mesh_norm_set(menu_Mesh* s, const int id, const menu_vector p);

// Adding prebuild stuff
void		menu_Mesh_add_tilemap_tile(menu_Mesh* m, menu_vector o, int32_t x, int32_t y, int32_t id);
void		menu_Mesh_add_tile(menu_Mesh* m, menu_vector p, int32_t id);
void		menu_Mesh_add_texture_part(menu_Mesh* m, menu_vector vtl, menu_texC ttl, menu_texC tbr);
void		menu_Mesh_add_rect(menu_Mesh* s, menu_vector vtl, menu_vector vbr);
void		menu_Mesh_add_sprite(menu_Mesh* m, menu_vector vtl, menu_vector vbr, menu_sprite_coord* sc);
void		menu_Mesh_add_sprite_boxed(menu_Mesh* m, menu_vector vtl, menu_vector vbr, menu_sprite_coord* sc, int b);
#if defined(__cplusplus)
}
#endif
#endif
