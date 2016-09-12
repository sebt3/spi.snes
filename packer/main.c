#include "theme.h"
#include "db.h"

void		theme_add_From_Dir(menu_Themes *thm, const char *dirname);
void		db_add_From_Dir(menu_db *d, const char *dirname, uint16_t images);
int main(int argc, char *argv[]) {
	menu_db *d;
	
	menu_db_Descriptor_Init();
	if (argc<3 || argv[1][0] != '-') {
		fprintf(stderr, "no file to pack %d - %s\n", argc, argv[1]);
		exit(1);
	}
	switch(argv[1][1]) {
	case 't':
		g = menu_Theme_new("theme");
		theme_add_From_Dir(g, argv[3]);
		menu_Theme_save(g, argv[2]);
		break;
	case 's':
		d = menu_db_new();
		db_add_From_Dir(d, argv[3], 0);
		menu_db_save(d, argv[2], EINA_TRUE);
		break;
	case 'm':
		d = menu_db_new();
		db_add_From_Dir(d, argv[3], 1);
		menu_db_save(d, argv[2], EINA_TRUE);
		break;
	default:
		fprintf(stderr, "Unknown option : %s\n", argv[1]);
		exit(2);
	}
	menu_db_Descriptor_Shutdown();
	return 0;
}
