#include <SDL/SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <Eina.h>
#include <Eet.h>
#include <unistd.h>
#include "theme.h"
#include "texture.h"
#include "mesh.h"

#define CHECK(x, y) if (x) {printf ("Malformed line %d at file %s : %s\n", lineno, p_file, y);continue;}
void	menu_Mesh_load_from_forms(char* name, char* p_file) {
	FILE *file = fopen ( p_file, "r" );
	menu_Mesh *ret=NULL;
	char line[40960], token[128], token2[128], sname[128], sprt[128], *pnt;
	int lineno=1;
	int x1,y1,x2,y2, b, res;
	menu_Texture *t = NULL;
	menu_sprite_coord* sc;
	menu_vector vtl = {0.0, 0.0, 0.05};
	menu_vector vbr = {0.0, 0.0, 0.05};
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		if (!strncmp(line, "from", 4)) {
			CHECK(sscanf(line, "%s %s", token, sname)<2, "");
			t = menu_Theme_find_Texture(g, sname);
			CHECK(t==NULL, "is not a texture");
		}else if(!strncmp(line, "sprt", 4)) {
			res = sscanf(line, "%s %s %s %d:%d %d:%d %s %d", token, sname, sprt, &x1, &y1, &x2, &y2, token2, &b);
			CHECK(res<7,"");
			vtl[0]=(float)x1;
			vtl[1]=(float)y1;
			vbr[0]=(float)x2;
			vbr[1]=(float)y2;
			sc = menu_Texture_get_sprite(t, sprt);
			CHECK(sc==NULL, "is not a sprite");
			if (res<9) {
				ret = menu_Mesh_new(sname, 4, 9, GL_TRIANGLE_STRIP);
				menu_Mesh_Texture_set(ret, t);
				menu_Mesh_add_sprite(ret, vtl, vbr, sc);
			}else {
				ret = menu_Mesh_new(sname, 4*9, 9*9, GL_TRIANGLE_STRIP);
				menu_Mesh_Texture_set(ret, t);
				menu_Mesh_add_sprite_boxed(ret, vtl, vbr, sc, b);
			}
			menu_Mesh_finalize(ret);
			menu_Theme_add_Mesh(g, ret);
		} else if(!strncmp(line, "text", 4)) {
			res = sscanf(line, "%s %s %s %d:%d %s", token, sname, sprt, &x1, &y1, token2);
			CHECK(res<6,"");
			vtl[0]=(float)x1;
			vtl[1]=(float)y1;
			CHECK(!(pnt = strchr(line, '-')),"no Text found");pnt++;
			menu_Theme_add_Text(g, menu_Text_new(sname,sprt,vtl,pnt));
		} else CHECK(EINA_FALSE, "");
		lineno++;
	}	
	fclose(file);
}
#undef CHECK

menu_Mesh *menu_Mesh_load_from_form(char* name, char* p_file) {
	FILE *file = fopen ( p_file, "r" );
	menu_Mesh *ret=NULL;
	char line[40960];
	char token[128];
	char token2[128];
	int lineno=0;
	int ln,bo,tx,ty,bx,by, res, b;
	menu_Texture *t;
	menu_sprite_coord* sc;
	menu_vector vtl = {0.0, 0.0, 0.05};
	menu_vector vbr = {0.0, 0.0, 0.05};
	if (file == NULL) return NULL;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		if (lineno==0 && sscanf(line, "%s %d %d", token, &ln, &bo)>1) {
			t = menu_Theme_find_Texture(g, token);
			if (t==NULL) {
				printf("Error while reading form(%s), texture(%s) not found. ignoring\n", p_file, token);
				break;
			}
			ret = menu_Mesh_new(name, 4*(ln+9*bo), 9*(ln+9*bo), GL_TRIANGLE_STRIP);
			menu_Mesh_Texture_set(ret, t);
		} else if (lineno>0&&lineno<ln+bo+1) {
			res = sscanf(line, "%s %d:%d %d:%d %s %d", token, &tx,&ty,&bx,&by,token2,&b); 
			if (res<5) {
				printf("Malformed line(%s,%d), ignoring\n", p_file, lineno+1);
				continue;
			}
			sc = menu_Texture_get_sprite(t, token);
			if (sc == NULL) {
				printf("Unknown sprite %s at %s:%d, ignoring\n",token, p_file, lineno+1);
				continue;
			}
			// add the sprite
			vtl[0]=(float)tx;
			vtl[1]=(float)ty;
			vbr[0]=(float)bx;
			vbr[1]=(float)by;
			if (res<7) {
				menu_Mesh_add_sprite(ret, vtl, vbr, sc);
			} else {
				menu_Mesh_add_sprite_boxed(ret, vtl, vbr, sc, b);
				
			}
		} else {
			printf("Malformed file(%s), ignoring\n", p_file);
			break;
		}
		lineno++;
	}	
	fclose(file);
	if (ret!=NULL)
		menu_Mesh_finalize(ret);
	return ret;
}

void		add_Font_From(menu_Themes *t, const char *filename, const char*parfile) {
	FILE *file = fopen ( parfile, "r" );
	char line[1024];
	char *token;
	int s;
	menu_Font *f;
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		token = calloc(128, sizeof(char));
		if (sscanf(line, "%s %d", token, &s)>0) {
			f = menu_Font_new(filename, s, token);
			menu_Theme_add_Texture(t, f->texture);
			menu_Theme_add_Texture(t, f->texture2);
			menu_Theme_add_Font(t, f);
		}else free(token);
	}
	fclose(file);
}

void		Texture_Properties_From(menu_Texture *texture, const char *parfile) {
	FILE *file = fopen ( parfile, "r" );
	char line[1024];
	char token[128];
	int x, y, a, b;
	menu_texC tl;
	menu_texC br;
	if (file == NULL) return;
	while ( fgets ( line, sizeof line, file ) != NULL ) {
		if (sscanf(line, "%s %d:%d %d:%d", token, &x, &y,&a,&b)<5) {
			b = sscanf(line, "%s %d:%d %d", token, &x, &y, &a);
			if (b>2) {
				if(!strcmp(token, "tilemap")) {
					texture->tilesheet = EINA_TRUE;
					texture->tile_w = x;
					texture->tile_h = y;
					if(b>3 && a==2)
						texture->scale = EINA_TRUE;
				}
				else
					printf("Malformed file(%s), ignoring\n", parfile);
				break;
			}
			break;
		}
		tl[0] = (float)x;
		tl[1] = (float)y;
		br[0] = (float)a;
		br[1] = (float)b;
		menu_Texture_add_sprite(texture,token,tl,br);
	}	
	fclose(file);
}


void		theme_add_From_Dir(menu_Themes *thm, const char *dirname) {
	DIR *dp;
	struct dirent *ep;
	struct stat buffer;
	menu_Texture *t;
	menu_Mesh *m;
	char *pnt, *parFile, *parF, *fullFile;

	dp = opendir (dirname);
	if (dp == NULL) return;
	// Load textures and fonts
	while ((ep = readdir (dp)))
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") && (pnt = strrchr(ep->d_name, '.'))) {
			fullFile = calloc(strlen(ep->d_name)+strlen(dirname)+2, sizeof(char));
			sprintf(fullFile, "%s/%s", dirname, ep->d_name);

			parFile = strdup(ep->d_name);
			memcpy(parFile + (int)pnt-(int)ep->d_name+1, "txt", 4);
			parF = calloc(strlen(parFile)+strlen(dirname)+2, sizeof(char));
			sprintf(parF, "%s/%s", dirname, parFile);free(parFile);

			if (stat(parF, &buffer)) {
				free(parF);parF=NULL;
			}

			if (!strncmp(pnt, ".bmp", 4)||!strncmp(pnt, ".png", 4)||!strncmp(pnt, ".jpg", 4)) {
				t = menu_Texture_load_from(ep->d_name, fullFile);
				if (parF != NULL) 
					Texture_Properties_From(t, parF);
				menu_Theme_add_Texture(thm, t);
			} else if (!strcmp(pnt, ".ttf")) {
				if (parF != NULL){
 					add_Font_From(thm, fullFile, parF);
				} else
					printf("Font : %s (NoParFile)\n", fullFile);

			}
			if (parF!=NULL)
				free(parF);
			free(fullFile);
		}
	rewinddir(dp);
	// Load forms and meshs
	while ((ep = readdir (dp)))
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") && (pnt = strrchr(ep->d_name, '.'))) {
			fullFile = calloc(strlen(ep->d_name)+strlen(dirname)+2, sizeof(char));
			sprintf(fullFile, "%s/%s", dirname, ep->d_name);

			if (!strcmp(pnt, ".frm")) {
				m = menu_Mesh_load_from_form(ep->d_name, fullFile);
				menu_Theme_add_Mesh(thm, m);
			}else if (!strcmp(pnt, ".frms")) {
				menu_Mesh_load_from_forms(ep->d_name, fullFile);
			}
			free(fullFile);
		}
	closedir (dp);
	
	printf("Found %d textures, %d fonts, %d mesh, %d texts\n"
		, eina_list_count(thm->textures)
		, eina_list_count(thm->fonts)
		, eina_list_count(thm->meshs)
		, eina_list_count(thm->texts)
	);

}
