#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

// Internal functions
void		menu_db_String_Free(menu_db* d, const char *str) {
	if (!str)
		return;

	if ( d->dict && eet_dictionary_string_check(d->dict, str))
		return;

	eina_stringshare_del(str);
}


void		 menu_Game_Genre_free(menu_db* db, menu_Game_Genre* g) {
	if (!g)		return;
	menu_db_String_Free(db, g->name);
	free(g);
}

void		 menu_Game_free(menu_db* db, menu_Game* g) {
	if (!g)		return;
	menu_db_String_Free(db, g->title);
	menu_db_String_Free(db, g->overView);
	free(g);
}

void		 menu_Crc_free(menu_Crc* c) {
	if (!c)		return;
	free(c);
}

void		 menu_File_free(menu_db* db, menu_File* f) {
	if (!f)		return;
	menu_db_String_Free(db, f->path);
	free(f);
}

menu_db*	 menu_db_new() {
	menu_db* ret = calloc(1, sizeof(menu_db));

	ret->genres	= NULL;
	ret->crc32	= NULL;
	ret->games	= NULL;
	ret->files	= NULL;
	ret->romDir[0]	= 0;

	return ret;
}

void		menu_db_free(menu_db* d) {

	menu_Game_Genre *t;
	menu_Game	*g;
	menu_Crc	*c;
	menu_File	*f;
	EINA_LIST_FREE(d->genres, t) {
		menu_Game_Genre_free(d, t);
	}
	EINA_LIST_FREE(d->crc32, c) {
		menu_Crc_free(c);
	}
	EINA_LIST_FREE(d->games, g) {
		menu_Game_free(d, g);
	}
	EINA_LIST_FREE(d->files, f) {
		menu_File_free(d, f);
	}
	free(d);
}

// FILES
menu_File*	 menu_db_add_File(menu_db *d, const char *p_path, const uint32_t gameId, const char *p_name, const uint32_t crc32) {
	menu_File*	ret	= calloc(1, sizeof(menu_File));
	ret->path		= eina_stringshare_add(p_path);
	ret->name		= eina_stringshare_add(p_name);
	ret->gameId		= gameId;
	ret->crc32		= crc32;
	d->files		= eina_list_append(d->files, ret);
	return ret;
	
}

menu_File*	 menu_db_find_File(menu_db *d, const uint32_t gameId) {
	Eina_List*	l;
	menu_File*	f;

	EINA_LIST_FOREACH(d->files, l, f) {
		if (f->gameId == gameId)
			return f;
	}
	return NULL;
}


void		 menu_db_remove_File(menu_db *d, const char *p_name) {
	Eina_List*	l, *l_next;
	menu_File*	f;

	EINA_LIST_FOREACH_SAFE(d->files, l, l_next, f) {
		if (!strcmp(f->path, p_name))
			d->files = eina_list_remove_list(d->files, l);
	}
}

void		 menu_db_empty_File(menu_db *d) {
	menu_File	*f;
	EINA_LIST_FREE(d->files, f) {
		menu_File_free(d, f);
	}
}

// Textures
menu_Texture*	 menu_db_add_Texture(menu_db *d, const char *p_name, const char *p_filename) {
	menu_Texture* ret	= menu_db_find_Texture(d, p_name);
	if (ret == NULL) {
		ret		= menu_Texture_load_from(p_name, p_filename);
		d->textures	= eina_list_append(d->textures, ret);
	}
	return ret;
}

menu_Texture*	 menu_db_find_Texture(menu_db *d, const char *p_name) {
	Eina_List*	l;
	menu_Texture*	t;

	EINA_LIST_FOREACH(d->textures, l, t) {
		if (!strcmp(t->name, p_name))
			return t;
	}
	return NULL;
}

void		 menu_db_remove_Texture(menu_db *d, const char *p_name) {
	menu_Texture* ret	= menu_db_find_Texture(d, p_name);
	if (ret == NULL) return;
	d->textures		= eina_list_remove(d->textures, ret);
	menu_Texture_free(ret);
}

void		menu_db_duplicate_Game_Genre(menu_db *d, menu_db *src) {
	Eina_List*		l;
	menu_Game_Genre*	g;

	EINA_LIST_FOREACH(src->genres, l, g) {
		if (!menu_db_find_Game_GenreId(d, g->name)) {
			menu_db_add_Game_Genre(d, g->genreId, g->name);
		}
	}
} 

//Configs
menu_Game_Config* menu_db_find_Game_Config(menu_db *d, uint32_t	 gameId) {
	Eina_List*		l;
	menu_Game_Config*	g;

	EINA_LIST_FOREACH(d->configs, l, g) {
		if (g->gameId == gameId)
			return g;
	}
	return NULL;
}

menu_Game_Config* menu_db_get_Game_Config(menu_db *d, uint32_t	 gameId) {
	menu_Game_Config* ret	= menu_db_find_Game_Config(d, gameId);
	if (ret == NULL) {
		ret		= calloc(1, sizeof(menu_Game_Config));
		ret->gameId	= gameId;
		d->configs	= eina_list_append(d->configs, ret);
	}
	return ret;
}

void		  menu_db_remove_Game_Config(menu_db *d,uint32_t gameId) {
	menu_Game_Config* ret	= menu_db_find_Game_Config(d, gameId);
	if (ret == NULL) return;
	d->configs		= eina_list_remove(d->configs, ret);
}


// Game Genre
menu_Game_Genre* menu_db_add_Game_Genre(menu_db *d, const uint16_t p_genreId, const char *p_name) {
	menu_Game_Genre* ret	= menu_db_find_Game_Genre(d, p_genreId);
	if (ret == NULL) {
		ret		= calloc(1, sizeof(menu_Game_Genre));
		ret->genreId	= p_genreId;
		ret->name	= eina_stringshare_add(p_name);
		d->genres	= eina_list_append(d->genres, ret);
	}
	return ret;
}

menu_Game_Genre* menu_db_find_Game_GenreId(menu_db *d, const char *p_name) {
	Eina_List*		l;
	menu_Game_Genre*	g;

	EINA_LIST_FOREACH(d->genres, l, g) {
		if (!strcmp(g->name, p_name))
			return g;
	}
	return NULL;
}

menu_Game_Genre* menu_db_find_Game_Genre(menu_db *d, const uint16_t p_genreId) {
	Eina_List*		l;
	menu_Game_Genre*	g;

	EINA_LIST_FOREACH(d->genres, l, g) {
		if (g->genreId == p_genreId)
			return g;
	}
	return NULL;
}

void		 menu_db_remove_Game_Genre(menu_db *d, const uint32_t p_genreId) {
	menu_Game_Genre* ret	= menu_db_find_Game_Genre(d, p_genreId);
	if (ret == NULL) return;
	d->genres		= eina_list_remove(d->genres, ret);
	menu_Game_Genre_free(d, ret);
}

void		 menu_db_copy_Game(menu_db *d, menu_Game* game, menu_db *src) {
	Eina_List*	l;
	menu_Crc*	c;
	menu_Texture*	t;
	menu_Texture*	t2;
	char txt[1024];
	int i;

	// duplicate the game itself
	menu_Game* r	= menu_db_add_Game(d, game->gameId);
	for(i=0;i<5;i++)
		r->genres[i]  = game->genres[i];
	r->title	= game->title;
	r->overView	= game->overView;

	// duplicate its CRC32
	EINA_LIST_FOREACH(src->crc32, l, c) {
		if (c->gameId == game->gameId)
			menu_db_add_Game_Crc(d, game->gameId, c->crc32);
	}

	// duplicate the texture
	sprintf(txt, "%8x/screen0.png", game->gameId);
	t = menu_db_find_Texture(src, txt);
	t2 = menu_db_find_Texture(d, txt);
	if (t!=NULL && t2==NULL) {
		d->textures	= eina_list_append(d->textures, t);
	}
}

menu_Game*	 menu_db_add_Game(menu_db *d, const uint32_t p_gameId) {
	menu_Game* 	ret	= menu_db_find_Game(d, p_gameId);
	if (ret == NULL) {
		ret		= calloc(1, sizeof(menu_Game));
		ret->gameId	= p_gameId;
		ret->genres[0]	= 0;
		ret->genres[1]	= 0;
		ret->genres[2]	= 0;
		ret->genres[3]	= 0;
		ret->genres[4]	= 0;
		ret->title	= NULL;
		d->games	= eina_list_append(d->games, ret);
	}
	return ret;
}

menu_Game*	 menu_db_find_Game(menu_db *d, const uint32_t p_gameId) {
	Eina_List*	l;
	menu_Game*	g;

	EINA_LIST_FOREACH(d->games, l, g) {
		if (g->gameId == p_gameId)
			return g;
	}
	return NULL;
}
menu_Game*	 menu_db_find_Game_byCrc(menu_db *d, const uint32_t p_Crc) {
	menu_Game*	ret = menu_db_find_Game(d, p_Crc);
	if (ret != NULL) return ret;
	menu_Crc*	c   = menu_db_find_Game_Crc(d, p_Crc);
	if (c == NULL) return NULL;
	return menu_db_find_Game(d, c->gameId);
}


void		 menu_db_remove_Game(menu_db *d, const uint32_t p_gameId) {
	menu_Game*	ret	= menu_db_find_Game(d, p_gameId);
	if (ret == NULL) return;
	d->games		= eina_list_remove(d->games, ret);
 	menu_Game_free(d, ret);
}

menu_Crc*	 menu_db_add_Game_Crc(menu_db *d, const uint32_t p_gameId, uint32_t p_Crc) {
	menu_Crc* ret	= menu_db_find_Game_Crc(d, p_Crc);
	if (ret == NULL) {
		ret		= calloc(1, sizeof(menu_Crc));
		ret->crc32	= p_Crc;
		ret->gameId	= p_gameId;
		d->crc32	= eina_list_append(d->crc32, ret);
	}
	return ret;
}

menu_Crc*	 menu_db_find_Game_Crc(menu_db *d, uint32_t p_Crc) {
	Eina_List*	l;
	menu_Crc*	c;

	EINA_LIST_FOREACH(d->crc32, l, c) {
		if (c->crc32 == p_Crc)
			return c;
	}
	return NULL;
}

void		 menu_db_remove_Game_Crc(menu_db *d, const uint32_t p_Crc) {
	menu_Crc*	ret	= menu_db_find_Game_Crc(d, p_Crc);
	if (ret == NULL) return;
	d->crc32		= eina_list_remove(d->crc32, ret);
	menu_Crc_free(ret);
}


static Eet_Data_Descriptor *	_menu_Texture_Descriptor;
static Eet_Data_Descriptor *	_menu_Config_Descriptor;
static Eet_Data_Descriptor *	_menu_Genre_Descriptor;
static Eet_Data_Descriptor *	_menu_Game_Descriptor;
static Eet_Data_Descriptor *	_menu_File_Descriptor;
static Eet_Data_Descriptor *	_menu_Crc_Descriptor;
static Eet_Data_Descriptor *	_menu_db_Descriptor;
static const char		_menu_db_Store[] = "default";

menu_db*	menu_db_load(const char *filename) {
	menu_db	*	ret = NULL;
	Eet_File	*ef = eet_open(filename, EET_FILE_MODE_READ);
	Eina_List*	l;
	menu_Texture*	t;
	unsigned int	w,h;
	int		a,c,q,lo;
	if (!ef) {
		fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
		return NULL;
	}

	ret	= eet_data_read(ef, _menu_db_Descriptor, _menu_db_Store);

	if (ret) {
		if (!ret->dict)
			ret->dict = eet_dictionary_get(ef);
		// Once the datatype evolve
		//if (ret->version < 2) {
			// do something to update the data ;)
		//}

		//Loading images
		EINA_LIST_FOREACH(ret->textures,l,t) {
			t->data = eet_data_image_read(ef, t->name, &w, &h, &a, &c, &q, &lo);
		}
	}

	eet_close(ef);
	return ret;
}

Eina_Bool	menu_db_save(const menu_db *db, const char *filename, Eina_Bool tex) {
	char		tmp[PATH_MAX];
	Eet_File *	ef;
	Eina_Bool	ret;
	unsigned int	i = 0, len;
	struct stat	st;
	Eina_List*	l;
	menu_Texture*	t;

	len = eina_strlcpy(tmp, filename, sizeof(tmp));
	if (tex) {
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
	}

	ef = eet_open(tmp, EET_FILE_MODE_READ_WRITE);
	if (!ef) {
		fprintf(stderr, "ERROR: could not open '%s' for write\n", tmp);
		return EINA_FALSE;
	}

	ret = eet_data_write(ef, _menu_db_Descriptor, _menu_db_Store, db, EET_COMPRESSION_VERYFAST);
	if (ret==0) {
		eet_close(ef);unlink(tmp);
		fprintf(stderr, "ERROR: could not write main data\n");
		return EINA_FALSE;
	}

	if (tex) {
		EINA_LIST_FOREACH(db->textures,l,t) {
			if (t->format == menu_TEXTURE_FORMAT_ALPHA)
				ret = eet_data_image_write (ef, t->name, t->data, t->w*sizeof(unsigned char)/sizeof(uint32_t), t->orig_h, 1, 3, 100, 0); //eet expect 32b data but we have something smaller
			else
				ret = eet_data_image_write (ef, t->name, t->data, t->w/2, t->orig_h, 1, 3, 100, 0); //eet expect 32b data but we have 16b hence the "w/2"
			if (!ret) {
				fprintf(stderr, "ERROR: could not write texture data : %s (%d)\n", t->name, i);
			}
		}
	}

	eet_close(ef);
	if (tex) {
		if (ret) {
			// Save succeded, moving to the real file
			unlink(filename);
			rename(tmp, filename);
		} else {
			// Save failed, removing garbage file
			unlink(tmp);
		}
	}
	return ret;
}


void		menu_db_Descriptor_Shutdown(void) {
	eet_data_descriptor_free(_menu_Config_Descriptor);
	eet_data_descriptor_free(_menu_Texture_Descriptor);
	eet_data_descriptor_free(_menu_Genre_Descriptor);
	eet_data_descriptor_free(_menu_Game_Descriptor);
	eet_data_descriptor_free(_menu_File_Descriptor);
	eet_data_descriptor_free(_menu_Crc_Descriptor);
	eet_data_descriptor_free(_menu_db_Descriptor);

	menu_Theme_Descriptor_Shutdown();
}

#define DB_ADD_LIST(member, eet_desc)			EET_DATA_DESCRIPTOR_ADD_LIST(_menu_db_Descriptor, menu_db, # member, member, eet_desc)
#define DB_ADD_BASIC(member, eet_type)			EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_db_Descriptor, menu_db, # member, member, eet_type)
#define DB_ADD_ARRAY(member, eet_type)			EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_menu_db_Descriptor, menu_db, # member, member, eet_type)
#define FILE_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_File_Descriptor, menu_File, # member, member, eet_type)
#define CRC_ADD_BASIC(member, eet_type)			EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Crc_Descriptor, menu_Crc, # member, member, eet_type)
#define GAME_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Game_Descriptor, menu_Game, # member, member, eet_type)
#define GAME_ADD_ARRAY(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC_ARRAY(_menu_Game_Descriptor, menu_Game, # member, member, eet_type)
#define GENRE_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Genre_Descriptor, menu_Game_Genre, # member, member, eet_type)
#define CONFIG_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Config_Descriptor, menu_Game_Config, # member, member, eet_type)
#define TEXTURE_ADD_BASIC(member, eet_type)		EET_DATA_DESCRIPTOR_ADD_BASIC(_menu_Texture_Descriptor, menu_Texture, # member, member, eet_type)
#define TEXTURE_ADD_LIST(member, eet_desc)		EET_DATA_DESCRIPTOR_ADD_LIST(_menu_Texture_Descriptor, menu_Texture, # member, member, eet_desc)

void		menu_db_Descriptor_Init(void) {
	Eet_Data_Descriptor_Class eddc;

	menu_Theme_Descriptor_Init();

	
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

	// Config
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Game_Config);
	_menu_Config_Descriptor = eet_data_descriptor_stream_new(&eddc);
	CONFIG_ADD_BASIC(gameId,	EET_T_INT);
	CONFIG_ADD_BASIC(proj,		EET_T_UCHAR);
	CONFIG_ADD_BASIC(cpuFilter,	EET_T_UCHAR);
	CONFIG_ADD_BASIC(gpuFilter,	EET_T_UCHAR);

	// Genre
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Game_Genre);
	_menu_Genre_Descriptor = eet_data_descriptor_stream_new(&eddc);
	GENRE_ADD_BASIC(genreId,	EET_T_USHORT);
	GENRE_ADD_BASIC(name,		EET_T_STRING);

	// Games
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Game);
	_menu_Game_Descriptor = eet_data_descriptor_stream_new(&eddc);
	GAME_ADD_BASIC(gameId,		EET_T_INT);
	GAME_ADD_ARRAY(genres,		EET_T_USHORT);
	GAME_ADD_BASIC(title,		EET_T_STRING);
	GAME_ADD_BASIC(name,		EET_T_STRING);
	GAME_ADD_BASIC(overView,	EET_T_STRING);

	// Crc32
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_Crc);
	_menu_Crc_Descriptor = eet_data_descriptor_stream_new(&eddc);
	CRC_ADD_BASIC(crc32,		EET_T_INT);
	CRC_ADD_BASIC(gameId,		EET_T_INT);

	// Files
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_File);
	_menu_File_Descriptor = eet_data_descriptor_stream_new(&eddc);
	FILE_ADD_BASIC(gameId,		EET_T_INT);
	FILE_ADD_BASIC(crc32,		EET_T_INT);
	FILE_ADD_BASIC(startCount,	EET_T_INT);
	FILE_ADD_BASIC(name,		EET_T_STRING);
	FILE_ADD_BASIC(path,		EET_T_STRING);

	// DB
	EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, menu_db);
	_menu_db_Descriptor = eet_data_descriptor_stream_new(&eddc);
	DB_ADD_LIST(configs,	_menu_Config_Descriptor);
	DB_ADD_LIST(genres,	_menu_Genre_Descriptor);
	DB_ADD_LIST(games,	_menu_Game_Descriptor);
	DB_ADD_LIST(crc32,	_menu_Crc_Descriptor);
	DB_ADD_LIST(files,	_menu_File_Descriptor);
	DB_ADD_LIST(textures,	_menu_Texture_Descriptor);
	DB_ADD_ARRAY(romDir,	EET_T_CHAR); // Generate a segfault at saving !
}
