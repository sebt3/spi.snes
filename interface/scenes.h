#ifndef __menu_SCENES_H__
#define __menu_SCENES_H__
#include "engine.h"
#include "db.h"

#if defined(__cplusplus)
extern "C" {
#endif

void		menu_Scenes_init(menu_db *db);


// EMU interfaces
extern menu_Texture *render;
void		EMU_Init();
menu_Texture*	EMU_getRenderer();
uint32_t	EMU_getCRC();
void		EMU_LoadRom(char *filename);
void		EMU_saveState(int slot);
void		EMU_loadState(int slot);
void		EMU_pause();
void		EMU_play();
void		EMU_deInit();
void		EMU_runFrame();
char *		EMU_getRomName();
void		EMU_onQuit();
void		EMU_processEvent(SDL_Event *e);
void		EMU_setVideoCfg(Eina_Bool p_linear, uint16_t p_proj, uint16_t p_cpu, uint16_t p_gpu);
void		EMU_resize(int width, int height);
#if defined(__cplusplus)
}
#endif
#endif
