#include <Eina.h>
#include <Eet.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "theme.h"
#include "texture.h"

menu_Themes *g = NULL;


menu_Themes*	menu_Theme_new(const char *p_name) {
	menu_Themes * ret = calloc(1, sizeof(menu_Themes));

	ret->version	= 1;
	ret->meshs	= NULL;
	ret->fonts	= NULL;
	ret->textures	= NULL;
	ret->texts	= NULL;
	ret->name	= eina_stringshare_add(p_name);

	return ret;
}

void		menu_Theme_free(menu_Themes *g) {
	menu_String_Free(g->name);

	menu_Texture *t;
	menu_Font  *f;
	menu_Mesh  *m;
	EINA_LIST_FREE(g->textures, t) {
		menu_Texture_free(t);
	}
	EINA_LIST_FREE(g->fonts, f) {
		menu_Font_free(f);
	}
	EINA_LIST_FREE(g->meshs, m) {
		menu_Mesh_free(m);
	}
	free(g);
}

void		menu_Theme_add_Texture(menu_Themes *game, const menu_Texture *texture) {
	if(texture!=NULL) game->textures = eina_list_append(game->textures, texture);
}

menu_Texture*	menu_Theme_find_Texture(const menu_Themes *game, const char *name) {
	Eina_List*	l;
	menu_Texture*	t;

	if (!game) return NULL;
	EINA_LIST_FOREACH(game->textures, l, t) {
		if (!strcmp(t->name, name))
			return t;
	}
	printf("Texture not found : %s\n", name);
	return NULL;
}

void		menu_Theme_remove_Texture(menu_Themes *game, const char *name) {
	menu_Texture*	t = menu_Theme_find_Texture(game, name);
	game->textures = eina_list_remove(game->textures, t);
	menu_Texture_free(t);
}



void		menu_Theme_add_Font(menu_Themes *game, const menu_Font *font) {
	if(font!=NULL) game->fonts = eina_list_append(game->fonts, font);
}

menu_Font*	menu_Theme_find_Font(const menu_Themes *game, const char *name) {
	Eina_List*	l;
	menu_Font*	f;

	if (!game) return NULL;
	EINA_LIST_FOREACH(game->fonts, l, f) {
		if (!strcmp(f->name, name))
			return f;
	}
	return NULL;
}

void		menu_Theme_remove_Font(menu_Themes *game, const char *name) {
	menu_Font*	t = menu_Theme_find_Font(game, name);
	game->fonts = eina_list_remove(game->fonts, t);
	menu_Font_free(t);
}



void		menu_Theme_add_Mesh(menu_Themes *game, const menu_Mesh *m) {
	if(m!=NULL) game->meshs = eina_list_append(game->meshs, m);
}

menu_Mesh*	menu_Theme_find_Mesh(const menu_Themes *game, const char *name) {
	Eina_List*	l;
	menu_Mesh*	m;

	EINA_LIST_FOREACH(game->meshs, l, m) {
		if (!strcmp(m->name, name))
			return m;
	}
	printf("Mesh %s not found\n", name);
	return NULL;
}

void		menu_Theme_remove_Mesh(menu_Themes *game, const char *name) {
	menu_Mesh*	t = menu_Theme_find_Mesh(game, name);
	game->meshs = eina_list_remove(game->meshs, t);
	menu_Mesh_free(t);
}



void		menu_Theme_add_Text(menu_Themes *game, const menu_Text *m) {
	if(m!=NULL) game->texts = eina_list_append(game->texts, m);
}

menu_Text*	menu_Theme_find_Text(const menu_Themes *game, const char *name) {
	Eina_List*	l;
	menu_Text*	m;

	EINA_LIST_FOREACH(game->texts, l, m) {
		if (!strcmp(m->name, name))
			return m;
	}
	//printf("Text %s not found\n", name);
	return NULL;
}

void		menu_Theme_remove_Text(menu_Themes *game, const char *name) {
	menu_Text*	t = menu_Theme_find_Text(game, name);
	game->texts = eina_list_remove(game->texts, t);
	menu_Text_free(t);
}

#define menu_STORAGE_USE_IMAGE_DATA

static Eet_Data_Descriptor *	_menu_Texture_Descriptor;
static Eet_Data_Descriptor *	_menu_Sprite_Descriptor;
static Eet_Data_Descriptor *	_menu_Font_stbtt_Descriptor;
static Eet_Data_Descriptor *	_menu_Font_Descriptor;
static Eet_Data_Descriptor *	_menu_Mesh_Descriptor;
static Eet_Data_Descriptor *	_menu_Text_Descriptor;
static Eet_Data_Descriptor *	_menu_Theme_Descriptor;
static const char		_menu_Theme_Store[] = "default";

static Eet_Dictionary *		_menu_Dict = NULL;

void		menu_String_Free(const char *str) {
	if (!str)
		return;

	if ( _menu_Dict && eet_dictionary_string_check(_menu_Dict, str))
		return;

	eina_stringshare_del(str);
}

menu_Themes*	menu_Theme_load(const char *filename) {
#ifdef menu_STORAGE_USE_IMAGE_DATA
	menu_Mesh*	m;
#endif
	Eina_List*	l;
	menu_Texture*	t;
	menu_Themes	*ret = NULL;//calloc(1, sizeof(menu_Themes));
	Eet_File	*ef  = eet_open(filename, EET_FILE_MODE_READ);
	unsigned int	w,h;
	int		a,c,q,lo;
	if (!ef) {
		fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
		return NULL;
	}

	ret	= eet_data_read(ef, _menu_Theme_Descriptor, _menu_Theme_Store);

	if (ret) {
		if (!_menu_Dict)
			_menu_Dict = eet_dictionary_get(ef);
		/* Once the datatype evolve
		if (ret->version < 2) {
			// do something to update the data ;)
		}
		*/

		EINA_LIST_FOREACH(ret->textures,l,t) {
			t->data = eet_data_image_read(ef, t->name, &w, &h, &a, &c, &q, &lo);
		}

#ifdef menu_STORAGE_USE_IMAGE_DATA
		EINA_LIST_FOREACH(ret->meshs,l,m) {
			int i=m->dots_count;
			m->dots = (float*)eet_read(ef, m->name, &c);
			m->loaded = EINA_TRUE;
			if (m->dots == NULL) {
				printf("Load mesh %s FAILED\n", m->name);
				break;
			}
			if (m->cols_count>0) {
				m->cols 	= m->dots + i;
				i+=m->cols_count;
			}
			if (m->tex_count>0) {
				m->tex		= m->dots + i;
				i+=m->tex_count;
			}
			if (m->norm_count>0) {
				m->norm		= m->dots + i;
				i+=m->norm_count;
			}
			m->faces		= (unsigned short*) (m->dots + i);
		}
#endif
	}

	eet_close(ef);
	return ret;
}

Eina_Bool	menu_Theme_save(const menu_Themes *game, const char *filename) {
	char		tmp[PATH_MAX];
	Eet_File *	ef;
	Eina_Bool	ret;
	unsigned int	i, len;
	struct stat	st;
	Eina_List*	l;
	menu_Texture*	t;
#ifdef menu_STORAGE_USE_IMAGE_DATA
	menu_Mesh*	m;
#endif

	len = eina_strlcpy(tmp, filename, sizeof(tmp));
	if (len + 12 >= (int)sizeof(tmp)) {
		fprintf(stderr, "ERROR: file name is too big: %s\n", filename);
		return EINA_FALSE;
	}

	// looking for a temporary filename
	i = 0;
	do {
		snprintf(tmp + len, 12, ".%u", i);
		i++;
	} while (stat(tmp, &st) == 0);

	ef = eet_open(tmp, EET_FILE_MODE_WRITE);
	if (!ef) {
		fprintf(stderr, "ERROR: could not open '%s' for write\n", tmp);
		return EINA_FALSE;
	}

	ret = eet_data_write(ef, _menu_Theme_Descriptor, _menu_Theme_Store, game, EET_COMPRESSION_VERYFAST);
	if (ret==0) {
		eet_close(ef);unlink(tmp);
		fprintf(stderr, "ERROR: could not write main data\n");
		return EINA_FALSE;
	}

	EINA_LIST_FOREACH(game->textures,l,t) {
 		//printf("Saving texture %s\n", t->name);
		if (t->format == menu_TEXTURE_FORMAT_ALPHA)
			ret = eet_data_image_write (ef, t->name, t->data, t->w*sizeof(unsigned char)/sizeof(uint32_t), t->orig_h, 1, 3, 100, 0); //eet expect 32b data but we have something smaller
		else
			ret = eet_data_image_write (ef, t->name, t->data, t->w/2, t->orig_h, 1, 3, 100, 0); //eet expect 32b data but we have 16b hence the "w/2"
		if (!ret) {
			eet_close(ef);unlink(tmp);
			fprintf(stderr, "ERROR: could not write texture data : %s\n", t->name);
			return EINA_FALSE;
		}
	}
#ifdef menu_STORAGE_USE_IMAGE_DATA
	EINA_LIST_FOREACH(game->meshs,l,m) {
 		int x		 = 3;
		int y;
		int i		 = m->dots_count;
		float *mem	 = NULL;
		if (m->cols!=NULL)	x+=4;
		if (m->tex!=NULL)	x+=2;
		if (m->norm!=NULL)	x+=3;
		x		*= 2;
		y		 = m->dots_count/3 + m->faces_count/(2*x)+1;
		mem		 = calloc(x*y,sizeof(float));
		
		
		memcpy(mem, m->dots, m->dots_count*sizeof(float));
		if (m->cols_count>0) {
			memcpy(mem+i, m->cols, m->cols_count*sizeof(float));
			i+=m->cols_count;
		}
		if (m->tex_count>0) {
			memcpy(mem+i, m->tex, m->tex_count*sizeof(float));
			i+=m->tex_count;
		}
		if (m->norm_count>0) {
			memcpy(mem+i, m->norm, m->norm_count*sizeof(float));
			i+=m->norm_count;
		}
		memcpy(mem+i, m->faces, m->faces_count*sizeof(unsigned short));

		ret = eet_write(ef, m->name, mem, x*y*sizeof(float), 1);
		free(mem);
		if (!ret) {
			eet_close(ef);unlink(tmp);
			fprintf(stderr, "ERROR: could not write mesh data : %s\n", m->name);
			return EINA_FALSE;
		}
	}
#endif


	eet_close(ef);
	if (ret) {
		// Save succeded, moving to the real file
		unlink(filename);
		rename(tmp, filename);
	} else {
		// Save failed, removing garbage file
		unlink(tmp);
	}
	return ret;
}


void		menu_Theme_Descriptor_Shutdown(void) {
	eet_data_descriptor_free(_menu_Text_Descriptor);
	eet_data_descriptor_free(_menu_Mesh_Descriptor);
	eet_data_descriptor_free(_menu_Font_stbtt_Descriptor);
	eet_data_descriptor_free(_menu_Font_Descriptor);
	eet_data_descriptor_free(_menu_Sprite_Descriptor);
	eet_data_descriptor_free(_menu_Texture_Descriptor);
	eet_data_descriptor_free(_menu_Theme_Descriptor);

	eet_shutdown();
	eina_shutdown();
}

#define THEME_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Theme_Descriptor, menu_Themes, # member, member, eet_type)
#define THEME_ADD_LIST(member, eet_desc)		EET_DATA_DESCRIPTOR_ADD_LIST(_menu_Theme_Descriptor, menu_Themes, # member, member, eet_desc)
#define TEXTURE_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Texture_Descriptor, menu_Texture, # member, member, eet_type)
#define TEXTURE_ADD_LIST(member, eet_desc)		EET_DATA_DESCRIPTOR_ADD_LIST(_menu_Texture_Descriptor, menu_Texture, # member, member, eet_desc)
#define SPRITE_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Sprite_Descriptor, menu_sprite_coord, # member, member, eet_type)
#define SPRITE_ADD_ARRAY(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_menu_Sprite_Descriptor, menu_sprite_coord, # member, member, eet_type)
#define STBTT_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Font_stbtt_Descriptor, stbtt_bakedchar, # member, member, eet_type)
#define FONT_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Font_Descriptor, menu_Font, # member, member, eet_type)
#define FONT_ADD_ARRAY(member, eet_desc)		EET_DATA_DESCRIPTOR_ADD_ARRAY(_menu_Font_Descriptor, menu_Font, # member, member, eet_desc)
#define MESH_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Mesh_Descriptor, menu_Mesh, # member, member, eet_type)
#define MESH_ADD_BASIC_VAR_ARRAY(member, eet_type)	EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(_menu_Mesh_Descriptor, menu_Mesh, # member, member, eet_type)
#define MESH_ADD_ARRAY(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_menu_Mesh_Descriptor, menu_Mesh, # member, member, eet_type)
#define TEXT_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Text_Descriptor, menu_Text, # member, member, eet_type)
#define TEXT_ADD_ARRAY(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_menu_Text_Descriptor, menu_Text, # member, member, eet_type)

void		menu_Theme_Descriptor_Init(void) {
	Eet_Data_Descriptor_Class eddc;

	eina_init();
	eet_init();
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Themes);
	_menu_Theme_Descriptor = eet_data_descriptor_stream_new(&eddc);

	// Sprite
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_sprite_coord);
	_menu_Sprite_Descriptor = eet_data_descriptor_stream_new(&eddc);

	SPRITE_ADD_BASIC(name,		EET_T_STRING);
	SPRITE_ADD_BASIC(sw,		EET_T_USHORT);
 	SPRITE_ADD_BASIC(sh,		EET_T_USHORT);
	SPRITE_ADD_ARRAY(tex,		EET_T_FLOAT);

	// Texture
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Texture);
	_menu_Texture_Descriptor = eet_data_descriptor_stream_new(&eddc);

	TEXTURE_ADD_BASIC(name,		EET_T_STRING);
	TEXTURE_ADD_BASIC(w,		EET_T_USHORT);
	TEXTURE_ADD_BASIC(h,		EET_T_USHORT);
	TEXTURE_ADD_BASIC(orig_w,	EET_T_USHORT);
	TEXTURE_ADD_BASIC(orig_h,	EET_T_USHORT);
	TEXTURE_ADD_BASIC(format,	EET_T_UCHAR);
	TEXTURE_ADD_BASIC(scale,	EET_T_CHAR);
	TEXTURE_ADD_BASIC(spritesheet,	EET_T_CHAR);
	TEXTURE_ADD_BASIC(tilesheet,	EET_T_CHAR);
	TEXTURE_ADD_BASIC(tile_w,	EET_T_USHORT);
 	TEXTURE_ADD_BASIC(tile_h,	EET_T_USHORT);
	TEXTURE_ADD_LIST(sprites,	_menu_Sprite_Descriptor);


	// Font
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Font);
	_menu_Font_Descriptor = eet_data_descriptor_stream_new(&eddc);
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, stbtt_bakedchar);
	_menu_Font_stbtt_Descriptor = eet_data_descriptor_stream_new(&eddc);
	STBTT_ADD_BASIC(x0,		EET_T_UINT);
	STBTT_ADD_BASIC(y0,		EET_T_UINT);
	STBTT_ADD_BASIC(x1,		EET_T_UINT);
	STBTT_ADD_BASIC(y1,		EET_T_UINT);
	STBTT_ADD_BASIC(xoff,		EET_T_FLOAT);
	STBTT_ADD_BASIC(yoff,		EET_T_FLOAT);
	STBTT_ADD_BASIC(xadvance,	EET_T_FLOAT);
	FONT_ADD_BASIC(name,		EET_T_STRING);
	FONT_ADD_BASIC(tex_name,	EET_T_STRING);
	FONT_ADD_ARRAY(cdata,		_menu_Font_stbtt_Descriptor);
	FONT_ADD_BASIC(tex_name2,	EET_T_STRING);
	FONT_ADD_ARRAY(cdata2,		_menu_Font_stbtt_Descriptor);

	// Text
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Text);
	_menu_Text_Descriptor = eet_data_descriptor_stream_new(&eddc);
	TEXT_ADD_BASIC(name,		EET_T_STRING);
	TEXT_ADD_BASIC(font,		EET_T_STRING);
	TEXT_ADD_BASIC(text,		EET_T_STRING);
	TEXT_ADD_ARRAY(pos,		EET_T_FLOAT);

	// Mesh
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Mesh);
	_menu_Mesh_Descriptor = eet_data_descriptor_stream_new(&eddc);
	MESH_ADD_BASIC(name,		EET_T_STRING);
	MESH_ADD_BASIC(texture_name,	EET_T_STRING);
	MESH_ADD_BASIC(mode,		EET_T_UCHAR);
	MESH_ADD_BASIC(alpha,		EET_T_UCHAR);
	MESH_ADD_BASIC(is_tile,		EET_T_UCHAR);
	MESH_ADD_BASIC(tilemap_w,	EET_T_USHORT);
	MESH_ADD_BASIC(tilemap_h,	EET_T_USHORT);
	MESH_ADD_ARRAY(col.t,		EET_T_FLOAT);
	MESH_ADD_ARRAY(ambiant.t,	EET_T_FLOAT);
	MESH_ADD_ARRAY(difuse.t,	EET_T_FLOAT);
	MESH_ADD_ARRAY(specular.t,	EET_T_FLOAT);
	MESH_ADD_ARRAY(transfilter.t,	EET_T_FLOAT);
	MESH_ADD_BASIC(illum,		EET_T_USHORT);
	MESH_ADD_BASIC(dots_count,	EET_T_INT);
	MESH_ADD_BASIC(cols_count,	EET_T_INT);
	MESH_ADD_BASIC(tex_count,	EET_T_INT);
	MESH_ADD_BASIC(norm_count,	EET_T_INT);
	MESH_ADD_BASIC(faces_count,	EET_T_INT);
#ifndef menu_STORAGE_USE_IMAGE_DATA
	MESH_ADD_BASIC_VAR_ARRAY(dots,	EET_T_FLOAT);
	MESH_ADD_BASIC_VAR_ARRAY(cols,	EET_T_FLOAT);
	MESH_ADD_BASIC_VAR_ARRAY(tex,	EET_T_FLOAT);
	MESH_ADD_BASIC_VAR_ARRAY(norm,	EET_T_FLOAT);
	MESH_ADD_BASIC_VAR_ARRAY(faces,	EET_T_USHORT);
#endif
	// Game
	THEME_ADD_BASIC(version,	EET_T_UINT);
	THEME_ADD_BASIC(name,	EET_T_STRING);
	THEME_ADD_LIST(textures,	_menu_Texture_Descriptor);
	THEME_ADD_LIST(fonts,	_menu_Font_Descriptor);
	THEME_ADD_LIST(meshs,	_menu_Mesh_Descriptor);
	THEME_ADD_LIST(texts,	_menu_Text_Descriptor);
}

