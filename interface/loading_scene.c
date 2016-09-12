#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "scenes.h"
#include "db.h"

static menu_db		*userDB, *systemDB;
static menu_Scene	*uiSc;
static menu_widget	*pct, *status1, *status2, *status3;
static Eina_List	*fileList;
static int		step, maxFound;
static char		stxt1[64], stxt2[64], stxt3[64];

static Eina_Bool isdir(const char *n) {
	struct stat s;
	if( stat(n,&s) == 0 ) {
		if( s.st_mode & S_IFDIR )
			return EINA_TRUE;
	}
	return EINA_FALSE;
}

static void addDir( const char *p_d) {
	DIR *dp;
	char *fullFile, *pnt;
	unsigned char isFolder =0x4;
	struct dirent *ep;

	dp = opendir (p_d);
	if (dp == NULL) return;
	while ((ep = readdir (dp)))
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
			//printf("(%x) %s\n", ep->d_type, ep->d_name);
			fullFile = calloc(strlen(ep->d_name)+strlen(p_d)+2, sizeof(char));
			sprintf(fullFile, "%s/%s", p_d, ep->d_name);
			if (ep->d_type == isFolder|| isdir(fullFile)) {
				addDir(fullFile);
				free(fullFile);
			}else if ((pnt = strrchr(ep->d_name, '.'))) {
				if (!strncmp(pnt, ".zip", 4)||!strncmp(pnt, ".ZIP", 4)||!strncmp(pnt, ".smc", 4)||!strncmp(pnt, ".SMC", 4)) {
					fileList = eina_list_append(fileList, fullFile);
				} else
					free(fullFile);
			} else
				free(fullFile);
		}
}

static void update_db(menu_Anime* a) {
	Eina_List *l;
	Eina_List *l_next;
	menu_widgetText *sw1 = (menu_widgetText *)status1->properties;
	menu_widgetText *sw2 = (menu_widgetText *)status2->properties;
	menu_widgetText *sw3 = (menu_widgetText *)status3->properties;
	char *data, *fname;
	menu_Game *game;
	int i=0;
	switch(step) {
	default:
	case 0:
		menu_db_empty_File(userDB);
		systemDB = menu_db_load("db_medium.eet");
		if(!systemDB) {
			menu_finish();
			exit(1);
		}
		menu_db_duplicate_Game_Genre(userDB, systemDB);
		menu_widgetProgress_setPct(pct, 5);
		a->tickLeft++;
		step=1;
	break;
	case 1:
		printf("ROMDIR= %s\n", userDB->romDir);
		addDir(userDB->romDir);
		maxFound = eina_list_count(fileList);
		menu_widgetProgress_setPct(pct, 10);
		a->tickLeft++;
		step++;
		// update labels
		sw1->text->text = stxt1;
		sw2->text->text = stxt2;
		sw3->text->text = stxt3;
		sprintf(stxt1, "files:%d", maxFound);
		menu_widgetScene_redraw(status1->wl);
	break;
	case 2:
		if (fileList && eina_list_count(fileList)>0 ) {
			i=0;
			EINA_LIST_FOREACH_SAFE(fileList, l, l_next, data) {
				fileList = eina_list_remove_list(fileList, l);
				EMU_LoadRom(data);
				if(menu_db_find_File(userDB, EMU_getCRC()))
					continue;
				game		= menu_db_find_Game_byCrc(systemDB, EMU_getCRC());
				fname = strrchr(data, '/');
				if (!fname) fname = data;
				else fname++;
				if (game !=NULL) {
					printf("found CRC\t: %X ### %s ### %s\n", EMU_getCRC(), EMU_getRomName(), fname);
					menu_db_copy_Game(userDB, game, systemDB);
				} else
					printf("unknown CRC\t: %X ### %s ### %s\n", EMU_getCRC(), EMU_getRomName(), fname);

				menu_db_add_File(userDB, data, (game?game->gameId:EMU_getCRC()), EMU_getRomName(), EMU_getCRC());
				i++;if (i>3) break; 
			}
			// update labels
			sprintf(stxt1, "files: %d", eina_list_count(fileList));
			sprintf(stxt2, "roms: %d", eina_list_count(userDB->files));
			sprintf(stxt3, "games: %d", eina_list_count(userDB->games));
			menu_widgetScene_redraw(status1->wl);
		} else step++;
		a->tickLeft++;
		menu_widgetProgress_setPct(pct, 10+((80*(maxFound-eina_list_count(fileList)))/(maxFound+1)));
	break;
	case 3:
		menu_db_save(userDB, "userdb.eet", EINA_TRUE);
		printf("Found %d games, %d textures, %d genres, %d crcs, %d files\n"
			, eina_list_count(userDB->games)
			, eina_list_count(userDB->textures)
			, eina_list_count(userDB->genres)
			, eina_list_count(userDB->crc32)
			, eina_list_count(userDB->files)
		);
		menu_widgetProgress_setPct(pct, 100);
		step = 0;
		menu_setUIScene(menu_findScene("simple"));
	break;
	}
}

void		menu_Scenes_loading_init(menu_db *db) {
	menu_Anime* a;
	fileList	= NULL;
	step 		= 0;
	userDB		= db;
	uiSc		= menu_widgetScene_new("load", EINA_FALSE);
	pct		= menu_widgetProgress_new(uiSc->wl, "pct", 0);
	a = menu_Scene_add_anime(uiSc, NULL);
	a->custom	= update_db;
	status1		= menu_widgetText_new(uiSc->wl, "status1", 3);
	status2		= menu_widgetText_new(uiSc->wl, "status2", 3);
	status3		= menu_widgetText_new(uiSc->wl, "status3", 3);
}
