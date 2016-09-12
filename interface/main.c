#include <SDL/SDL.h>
#include <stdio.h>
#include <Eina.h>
#include <Eet.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "db.h"
#include "engine.h"
#include "scenes.h"
/*
- Emulation :
 * sauvegardes des configs avec config/jeu en plus de la config globale
- Widgets
 * drag&drop support pour le slider de selectList
- Ecrans :
 * Selection par screenshots (tableau de screenies qui scroll de gauche à droite)
 * Configuration des contrôles
- DB:
 * Cleanup
*/

int main(int argc, char *argv[]) {
	menu_db	 *userDB;
	Eina_Bool isNew = EINA_TRUE;

#ifdef USE_RENDER_THREAD
# if defined(__linux__)
	XInitThreads();
# endif
#endif

	menu_init();

	// Theme load
	menu_main_Loading("Loading theme...", 15);
	g = menu_Theme_load("theme.eet");
	if(!g) {
		printf("Failed to load theme file\n");
		menu_finish();
		return 1;
	}
	menu_main_Loading("Loading textures...", 40);
	menu_Texture_Theme_init(g);
	
	// Database Load
	menu_main_Loading("Loading database...", 50);
	menu_db_Descriptor_Init();
	userDB = menu_db_load("userdb.eet");
	if (!userDB) {
		userDB = menu_db_new();
		//TODO: add default settings
	} else
		isNew = EINA_FALSE;

	menu_main_Loading("Starting emulator engine", 70);
	EMU_Init();

	menu_main_Loading("setting up scenes", 90);
	menu_Scenes_init(userDB);
	menu_setGameScene(menu_findScene("simpleBG"));
	if (isNew)
		menu_setUIScene(menu_findScene("dirc"));
	else if (argc>1) {
		EMU_LoadRom(argv[1]);
		if (EMU_getCRC())
			menu_setUIScene(menu_findScene("EMU_UI"));
		else
			menu_setUIScene(menu_findScene("simple"));
	} else
		menu_setUIScene(menu_findScene("simple"));

	menu_main_Loop();

	EMU_onQuit();
	menu_db_save(userDB, "userdb.eet", EINA_FALSE);
	menu_Scenes_free();
	EMU_deInit();
	menu_finish();
	return 0;
}
