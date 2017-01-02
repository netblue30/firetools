#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../common/common.h"

int arg_debug = 0;
int arg_test = 0;

#define BUFSIZE 1024
static char buf[BUFSIZE];
static char *buf_title = NULL;
static char *buf_section = NULL;
static char *buf_command = NULL;
static char *buf_needs = NULL;

void process_line(char *line) {
	char *ptr;
	
	if ((ptr = strstr(line, "section=\"")) ||
	    (ptr = strstr(line, "Section=\""))) {
		ptr += 9;
		char *end = strchr(ptr, '"');
		if (end) { 
			*end = '\0';
			if (buf_section)
				free(buf_section);
			buf_section = strdup(ptr);
			if (!buf_section)
				errExit("strdup");
			*end = '"';
		}
	}

	if ((ptr = strstr(line, "command=\""))) {
		ptr += 9;
		char *end = strchr(ptr, '"');
		if (end) {
			*end = '\0';
			if (buf_command)
				free(buf_command);
			buf_command = strdup(ptr);
			if (!buf_command)
				errExit("strdup");
			*end = '"';
		}
	}
	
	if (strstr(line, "longtitle") == NULL && (ptr = strstr(line, "title=\"")) != NULL) {
		ptr += 7;
		char *end = strchr(ptr, '"');
		if (end) {
			*end = '\0';
			if (buf_title)
				free(buf_title);
			buf_title = strdup(ptr);
			if (!buf_title)
				errExit("strdup");
			*end = '"';
		}
	}
	
	if ((ptr = strstr(line, "needs=\""))) {
		ptr += 7;
		char *end = strchr(ptr, '"');
		if (end) {
			*end = '\0';
			if (buf_needs)
				free(buf_needs);
			buf_needs = strdup(ptr);
			if (!buf_needs)
				errExit("strdup");
			*end = '"';
		}
	}
}

void section_cleanup(void) {
	assert(buf_section);
	
	if (strncmp(buf_section, "Applications/", 13) == 0) {
		char *tmp = strdup(buf_section + 13);
		if (!tmp)
			errExit("strdup");
		
		free(buf_section);
		buf_section = tmp;
	}
	if (strncmp(buf_section, "Apps/", 5) == 0) {
		char *tmp = strdup(buf_section + 5);
		if (!tmp)
			errExit("strdup");
		
		free(buf_section);
		buf_section = tmp;
	}

	char *ptr = strchr(buf_section, '/');
	if (ptr)
		*ptr = '\0';
}

static void clean_buffers(void) {
	if (buf_section) {
		free(buf_section);
		buf_section = NULL;
	}
	if (buf_command) {
		free(buf_command);
		buf_command = NULL;
	}
	if (buf_title) {
		free(buf_title);
		buf_title = NULL;
	}
	if (buf_needs) {
		free(buf_needs);
		buf_needs = NULL;
	}
}

static void usage(void) {
	printf("Usage: debmenu [options]\n");
	printf("Option:\n");
	printf("\t--debug - enable debugging\n");
	printf("\t--test - test the files\n");
	printf("--help, -? - this help screen\n");
}

int main(int argc, char **argv) {
	// parse arguments
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--debug") == 0)
			arg_debug = 1;
		else if (strcmp(argv[i], "--test") == 0)
			arg_test = 1;
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		}
#if 0
		else if (strcmp(argv[i], "--version") == 0) {
			printf("Firejail-ui version " PACKAGE_VERSION "\n");
			return 0;
		}
#endif
		else {
			fprintf(stderr, "Error: invalid option\n");
			usage();
			return 1;
		}
	}
	
	
	
	
	DIR *dir = opendir("/usr/share/menu");
	if (!dir) {
		fprintf(stderr, "Error: cannot open /usr/share/menu\n");
		exit(1);
	}
	
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		// reject all dot files
		if (*entry->d_name == '.')
			continue;

		clean_buffers();
//		if (arg_test)
//			printf("processing %s\n", entry->d_name);
		
		// build filename
		char *fname;
		if (asprintf(&fname, "/usr/share/menu/%s", entry->d_name) == -1)
			errExit("asprintf");
		FILE *fp = fopen(fname, "r");
		free(fname);
		
		if (fp) {
			int printed = 0;
			// parse file
			while(fgets(buf, BUFSIZE, fp)) {
				char *ptr = buf;
				while (*ptr == ' ' || *ptr == '\t')
					ptr++;
					
				process_line(ptr);
				
				// print only x11 applications
				if (buf_section && buf_command && buf_title && buf_needs) {
					if (strcmp(buf_needs, "x11") == 0 || strcmp(buf_needs, "X11") == 0) {
						section_cleanup();
						if (!arg_test)
							printf("%s;%s;%s\n", buf_section, buf_title, buf_command);
						printed = 1;
					}
					else if (arg_test)
						printed = 1;
					clean_buffers();
				}
					
			}
			
			if (!printed && arg_test)
				fprintf(stderr, "Error: cannot process %s\n", entry->d_name);
		}
		
	}
	
	closedir(dir);
	return 0;
}
