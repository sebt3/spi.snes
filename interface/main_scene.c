#include "scenes.h"
#include "db.h"
#include "filters.h"

/***********************************************************************************
 *				Global scenes
 ***********************************************************************************/
void		menu_Scenes_main_init(menu_db *db);
void		menu_Scenes_loading_init(menu_db *db);
void		menu_Scenes_simple_init(menu_db *db);
void		menu_Scenes_inGame_init(menu_db *db);
void		menu_Scenes_cfgVideo_init(menu_db *db);
void		menu_Scenes_choosedir_init(menu_db *db);
void		menu_Scenes_cfgSelect_init(menu_db *db);

void		menu_Scenes_init(menu_db *db) {
	menu_Scenes_main_init(db);
	menu_Scenes_simple_init(db);
	menu_Scenes_inGame_init(db);
	menu_Scenes_cfgVideo_init(db);
	menu_Scenes_choosedir_init(db);
	menu_Scenes_loading_init(db);
	menu_Scenes_cfgSelect_init(db);
}


/***********************************************************************************
 *				Main scene
 ***********************************************************************************/
static menu_db		*DB;
static menu_Scene	*gameSc,   *uiSc;
static menu_RGroup		   *rom;
static menu_Segment	*seg;
static Eina_Bool	linear;
static int curshader, curfilter, curproj;

void		menu_Scene_main_prepare(menu_Scene *s) {
	menu_setGameScene(menu_findScene("EMU_BG"));
	EMU_play();
}

void		menu_Scene_main_dispose(menu_Scene *s) {	
	EMU_pause();
}

void menu_Scene_main_runEMU(menu_Anime* a) {
	EMU_runFrame();
	a->tickLeft++;
}

void	EMU_resize(int width, int height) {
	rom->t->orig_w = width;
	rom->t->orig_h = height;
	menu_Segment_sprite_setProj(seg,curproj);
}

void	EMU_setVideoCfg(Eina_Bool p_linear, uint16_t p_proj, uint16_t p_cpu, uint16_t p_gpu) {
	linear		= p_linear;
	curproj		= p_proj;
	if (menu_filter_get(p_cpu))
		curfilter	= p_cpu;
	if (curshader<SHADER_MAX_ID)
		curshader	= p_gpu;
	rom->t->filter	= menu_filter_get(curfilter);
	rom->shader	= menu_Shader_get_filter(curshader);
	menu_Segment_sprite_setProj(seg,curproj);
}

void	menu_Event_Handle_main(menu_Scene *s, SDL_Event *e) {
	switch(e->type) {
	case SDL_QUIT:
		menu_quit = EINA_TRUE;
	break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_q:
		case SDLK_ESCAPE:
			menu_setUIScenePrevious();
		break;
		case SDLK_n:
			menu_setUIScene(menu_findScene("cfgv"));
		break;
		case SDLK_m:
			menu_setUIScene(menu_findScene("menu"));
		break;
		default:
			EMU_processEvent(e);
		break;
		}
	break;
	default:
		EMU_processEvent(e);
	break;
	}
}

void		menu_Scenes_main_init(menu_db *db) {
	menu_Anime* a;
	linear		= EINA_FALSE;
	uiSc		= menu_Scene_new("EMU_UI");
	gameSc		= menu_Scene_new("EMU_BG");
	DB		= db;
	rom		= menu_RGroupSprite_new(1, EINA_FALSE, EINA_TRUE);
	curshader	= 0;
	curproj		= 0;
	curfilter	= 0;

	// UI setup
	uiSc->isEmu	= EINA_TRUE;
	uiSc->onEvent	= menu_Event_Handle_main;
	uiSc->prepare	= menu_Scene_main_prepare;
	uiSc->dispose	= menu_Scene_main_dispose;
	
	// game display setup
	menu_Scene_add_rgroup(gameSc, rom);
	menu_RGroup_Texture_set(rom, render);
	seg = menu_RGroup_Image_add(rom, 0,0, 800, 480);
	a = menu_Scene_add_anime(uiSc, NULL);
	a->custom	= menu_Scene_main_runEMU;
	
	// apply video config
	menu_Game_Config *cfg = menu_db_get_Game_Config(db, 0);
	EMU_setVideoCfg(EINA_FALSE, cfg->proj, cfg->cpuFilter, cfg->gpuFilter);
}
