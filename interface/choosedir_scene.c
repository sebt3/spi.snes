#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "scenes.h"
#include "db.h"

static menu_db		*userDB;
static menu_Scene	*uiSc;
static menu_widget	*sel;
static menu_widgetSelectItem dirs[4096];
static char currentPath[1024];
static char strtmp[1024];

static int	isDirectory(const struct dirent * ep) {
	char fullFile[2048];
	struct stat s;
	if (ep->d_type == 0x4) return EINA_TRUE;

	sprintf(fullFile, "%s/%s", currentPath, ep->d_name);
	if( stat(fullFile,&s) == 0 ) {
		if( s.st_mode & S_IFDIR )
			return EINA_TRUE;
	}
	return EINA_FALSE;
}

static void	setDirectory(const char *p_dir, const char *p_dir2) {
	sprintf(strtmp, "%s/%s", p_dir, p_dir2);
	realpath(strtmp, currentPath);
	struct dirent **namelist;
	int i		= 0;
	int n;

	n = scandir(currentPath, &namelist, isDirectory, alphasort);
	if (n > 0) {
		for (i=0;i<n;i++) {
			dirs[i].name  = calloc(strlen(namelist[i]->d_name)+1, sizeof(char));
			strcpy(dirs[i].name, namelist[i]->d_name);
			free(namelist[i]);
		}
		free(namelist);
	}

	menu_widgetSelectList_setItems(sel, dirs, i);
}

static void 	enterDir(menu_widget* w) {
	menu_widgetSelectList *p= w->properties;
	setDirectory(currentPath,p->items[p->current].name);
}

static void 	confirmDir(menu_widget* w) {
	//printf("%s\n",currentPath);
	sprintf(userDB->romDir, "%s", currentPath);
	menu_setUIScene(menu_findScene("load"));
}

void		menu_Scenes_choosedir_init(menu_db *db) {
	userDB		= db;
	uiSc		= menu_widgetScene_new("dirc", EINA_TRUE);
	sel		= menu_widgetSelectList_new(uiSc->wl, "dir", enterDir, NULL);
	menu_widgetButton_new(uiSc->wl, "ok",	confirmDir);
	menu_widgetScene_setSelected(uiSc->wl, sel);
	setDirectory(".", "");
}
