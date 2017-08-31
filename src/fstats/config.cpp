#include "fstats.h"
#include "../common/utils.h"

#define DEFAULT_X_SIZE 650
#define DEFAULT_Y_SIZE 650
#define MINSIZE 400
#define BUFSIZE 4096

void config_read_screen_size(int *x, int *y) {
	// set defaults
	*x = DEFAULT_X_SIZE;
	*y = DEFAULT_X_SIZE;

	// open config file
	char *cfgdir = get_config_directory();
	if (!cfgdir)
		return;
	char *fname;
	if (asprintf(&fname, "%s/fstats.config", cfgdir) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "r");
	free(fname);
	if (!fp)
		return;
	
	// read file and parse it
	char buf[BUFSIZE];
	while (fgets(buf, BUFSIZE, fp)) {
		char *ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (strncmp(ptr, "x ", 2) == 0) {
			ptr += 2;
			if (sscanf(ptr, "%d", x) != 1) {
				fprintf(stderr, "Error: invalid X size in ~/.config/firetools/fstats.config\n");
				return;
			}
		}
		else if (strncmp(ptr, "y ", 2) == 0) {
			ptr += 2;
			if (sscanf(ptr, "%d", y) != 1) {
				fprintf(stderr, "Error: invalid Y size in ~/.config/firetools/fstats.config\n");
				return;
			}
		}
	}
	fclose(fp);
}

void config_write_screen_size(int x, int y) {
	x = (x < MINSIZE)? DEFAULT_X_SIZE: x;
	y = (y < MINSIZE)? DEFAULT_Y_SIZE: y;
	
	// open config file
	char *cfgdir = get_config_directory();
	if (!cfgdir)
		return;
	char *fname;
	if (asprintf(&fname, "%s/fstats.config", cfgdir) == -1)
		errExit("asprintf");
	FILE *fp = fopen(fname, "w");
	free(fname);
	if (!fp)
		return;

	// write file
	fprintf(fp, "x %d\n", x);
	fprintf(fp, "y %d\n", y);
	fclose(fp);
}
