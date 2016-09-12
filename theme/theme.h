#ifndef __menu_GAME_H__
#define __menu_GAME_H__

#include <Eina.h>
#include "texture.h"
#include "mesh.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	unsigned int	engine_version;
	unsigned int	version;
  	const char*	name;
	Eina_List*	textures;
	Eina_List*	fonts;
	Eina_List*	meshs;
	Eina_List*	texts;
} menu_Themes;

menu_Themes*	menu_Theme_new(const char *p_name);
void		menu_Theme_free(menu_Themes *g);

void		menu_Theme_add_Texture(menu_Themes *game, const menu_Texture *texture);
menu_Texture*	menu_Theme_find_Texture(const menu_Themes *game, const char *name);
void		menu_Theme_remove_Texture(menu_Themes *game, const char *name);

void		menu_Theme_add_Font(menu_Themes *game, const menu_Font *font);
menu_Font*	menu_Theme_find_Font(const menu_Themes *game, const char *name);
void		menu_Theme_remove_Font(menu_Themes *game, const char *name);

void		menu_Theme_add_Mesh(menu_Themes *game, const menu_Mesh *m);
menu_Mesh*	menu_Theme_find_Mesh(const menu_Themes *game, const char *name);
void		menu_Theme_remove_Mesh(menu_Themes *game, const char *name);

void		menu_Theme_add_Text(menu_Themes *game, const menu_Text *m);
menu_Text*	menu_Theme_find_Text(const menu_Themes *game, const char *name);
void		menu_Theme_remove_Text(menu_Themes *game, const char *name);

void		menu_String_Free(const char *str);
void		menu_Theme_Descriptor_Init(void);
void		menu_Theme_Descriptor_Shutdown(void);

menu_Themes*	menu_Theme_load(const char *filename);
Eina_Bool	menu_Theme_save(const menu_Themes *game, const char *filename);

extern menu_Themes *g;


#if defined(__cplusplus)
}
#endif
#endif
