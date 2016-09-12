#ifndef __menu_DB_H__
#define __menu_DB_H__

#include <Eina.h>
#include <Eet.h>
#include "theme.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	uint16_t	genreId;
	const char	*name;
} menu_Game_Genre;


typedef struct {
	uint32_t	gameId;
	uint16_t	genres[5];
	const char	*title;
	const char	*name;
	const char	*overView;
} menu_Game;

typedef struct {
	uint32_t	gameId;
	uint8_t		proj;
	uint8_t		cpuFilter;
	uint8_t		gpuFilter;
} menu_Game_Config;


typedef struct {
	uint32_t	crc32;
	uint32_t	gameId;
} menu_Crc;


typedef struct {
	uint32_t	gameId;
	uint32_t	crc32;
	uint32_t	startCount;
	const char	*name;
  	const char*	path;
} menu_File;

typedef struct {
	Eet_Dictionary *dict;

	// system db
	Eina_List*	configs;
	Eina_List*	genres;
	Eina_List*	crc32;
	Eina_List*	games;
	Eina_List*	textures;

	// user db
	char		romDir[4096];
	Eina_List*	files;
} menu_db;

void		 menu_db_Descriptor_Init(void);
void		 menu_db_Descriptor_Shutdown(void);
menu_db*	 menu_db_new();
void		 menu_db_free(menu_db *d);
menu_db*	 menu_db_load(const char *filename);
Eina_Bool	 menu_db_save(const menu_db *db, const char *filename, Eina_Bool tex);

// Configs
menu_Game_Config* menu_db_find_Game_Config(menu_db *d, uint32_t	 gameId);
menu_Game_Config* menu_db_get_Game_Config(menu_db *d, uint32_t	 gameId);
void		  menu_db_remove_Game_Config(menu_db *d,uint32_t gameId);
// Genres
menu_Game_Genre* menu_db_add_Game_Genre(menu_db *d, const uint16_t p_genreId, const char *p_name);
menu_Game_Genre* menu_db_find_Game_Genre(menu_db *d, const uint16_t p_genreId);
menu_Game_Genre* menu_db_find_Game_GenreId(menu_db *d, const char *p_name);
void		 menu_db_remove_Game_Genre(menu_db *d, const uint32_t p_genreId);
void		 menu_db_duplicate_Game_Genre(menu_db *d, menu_db *src);
// Games
menu_Game*	 menu_db_add_Game(menu_db *d, const uint32_t p_gameId);
menu_Game*	 menu_db_find_Game(menu_db *d, const uint32_t p_gameId);
menu_Game*	 menu_db_find_Game_byCrc(menu_db *d, const uint32_t p_Crc);
void		 menu_db_remove_Game(menu_db *d, const uint32_t p_gameId);
void		 menu_db_copy_Game(menu_db *d, menu_Game* game, menu_db *src);
// CRC32s
menu_Crc*	 menu_db_add_Game_Crc(menu_db *d, const uint32_t p_gameId, uint32_t p_Crc);
menu_Crc*	 menu_db_find_Game_Crc(menu_db *d, uint32_t p_Crc);
void		 menu_db_remove_Game_Crc(menu_db *d, const uint32_t p_Crc);
// Textures
menu_Texture*	 menu_db_add_Texture(menu_db *d, const char *p_name, const char *p_filename);
menu_Texture*	 menu_db_find_Texture(menu_db *d, const char *p_name);
void		 menu_db_remove_Texture(menu_db *d, const char *p_name);

// Files
menu_File*	 menu_db_add_File(menu_db *d, const char *p_path, const uint32_t gameId, const char *p_name, const uint32_t crc32);
menu_File*	 menu_db_find_File(menu_db *d, const uint32_t gameId);
void		 menu_db_remove_File(menu_db *d, const char *p_name);
void		 menu_db_empty_File(menu_db *d);

#if defined(__cplusplus)
}
#endif
#endif