#include <SDL/SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include "db.h"

inline uint32_t	crcStr2Int(const char *str) {
	uint32_t ret;
	sscanf(str, "%x", &ret);
	return ret;
}


void		db_load_Genres(menu_db *d, const char *filename) {
	FILE	*file		= fopen ( filename, "r" );
	int	 genre		= 1;
	char	 line[1024];
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		//Add Genres
		menu_db_add_Game_Genre(d, genre, line);
		genre++;
	}
	fclose(file);
}
void		db_add_GameCrc_From(menu_db *d, const char *filename, const uint32_t gameId) {
	FILE	*file		= fopen ( filename, "r" );
	char	 line[1024];
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		menu_db_add_Game_Crc(d, gameId, crcStr2Int(line));
	}
	fclose(file);
}
void		db_add_GameGenres_From(menu_db *d, const char *filename, menu_Game *g) {
	FILE	*file		= fopen ( filename, "r" );
	menu_Game_Genre *genre	= NULL;
	int	 i		= 0;
	char	 line[1024];
	if (file  == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL && i<5) {
		genre = menu_db_find_Game_GenreId(d, line);
		if (genre == NULL) continue;
		g->genres[i] = genre->genreId;
		i++;
	}
	fclose(file);
}
void		db_set_GameTitle_From(menu_db *d, const char *filename, menu_Game *g) {
	FILE	*file		= fopen ( filename, "r" );
	char	 line[1024];
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL) {
		g->title = eina_stringshare_add(line);
		break;
	}
	fclose(file);
}

void		db_set_GameName_From(menu_db *d, const char *filename, menu_Game *g) {
	FILE	*file		= fopen ( filename, "r" );
	char	 line[1024];
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL) {
		g->name = eina_stringshare_add(line);
		break;
	}
	fclose(file);
}

void		db_add_GameDesc_From(menu_db *d, const char *filename, menu_Game *g) {
	FILE	*file		= fopen ( filename, "r" );
	char	 line[40960];
	if (file == NULL) return;
	size_t newLen = fread(line, sizeof(char), 40959, file);
	if (newLen != 0) {
		line[++newLen] = '\0'; /* Just to be safe. */
		g->overView = eina_stringshare_add(line);
	}
	fclose(file);
}
static int textures_count;
void		db_add_GameScreen_From(menu_db *d, const char *filename, const uint32_t gameId) {
	char	name[1024];
	// TODO: remove the 50 cap, fix the loading problem
	//if (textures_count++>50) return;
	sprintf(name, "%8x/screen0.png", gameId);
	menu_db_add_Texture(d, name, filename);
}

void		db_add_game_From_Dir(menu_db *d, const char *dirname, const char *gname, uint16_t images) {
	DIR		*dp;
	struct dirent	*ep;
	char 		*fullFile;
	uint32_t	 gameId = crcStr2Int(gname);
	menu_Game	*game   = menu_db_add_Game(d, gameId);

	dp = opendir (dirname);
	if (dp == NULL) return;
	while ((ep = readdir (dp)))
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
			fullFile = calloc(strlen(ep->d_name)+strlen(dirname)+2, sizeof(char));
			sprintf(fullFile, "%s/%s", dirname, ep->d_name);
			if (!strncmp(ep->d_name, "title", 5)) 
				db_set_GameTitle_From(d, fullFile, game);
			else if (!strncmp(ep->d_name, "romname", 7)) 
				db_set_GameName_From(d, fullFile, game);
			else if (!strncmp(ep->d_name, "CRC32", 5))
				db_add_GameCrc_From(d, fullFile, gameId);
			else if (!strncmp(ep->d_name, "genres", 6))
				db_add_GameGenres_From(d, fullFile, game);
			else if (!strncmp(ep->d_name, "description", 11))
				db_add_GameDesc_From(d, fullFile, game);
			else if (!strncmp(ep->d_name, "screen0.png", 11) && images>0)
				db_add_GameScreen_From(d, fullFile, gameId);
			free(fullFile);
		}
	closedir (dp);
}


void		db_add_From_Dir(menu_db *d, const char *dirname, uint16_t images) {
	DIR *dp;
	unsigned char isFolder =0x4;
	struct dirent *ep;
	textures_count=0;

	// Load genres first
	char *fullFile = calloc(strlen(dirname)+16, sizeof(char));
	sprintf(fullFile, "%s/genres", dirname);
	db_load_Genres(d, fullFile);
	free(fullFile);


	dp = opendir (dirname);
	if (dp == NULL) return;
	while ((ep = readdir (dp)))
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
			fullFile = calloc(strlen(ep->d_name)+strlen(dirname)+2, sizeof(char));
			sprintf(fullFile, "%s/%s", dirname, ep->d_name);
			if (ep->d_type == isFolder) {
				db_add_game_From_Dir(d, fullFile, ep->d_name, images);
			}
			free(fullFile);
		}

	closedir (dp);
	printf("Found %d games, %d textures, %d genres, %d crcs\n"
		, eina_list_count(d->games)
		, eina_list_count(d->textures)
		, eina_list_count(d->genres)
		, eina_list_count(d->crc32)
	);
}
