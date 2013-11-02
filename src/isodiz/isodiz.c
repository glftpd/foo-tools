
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <lib/sfv.h>
#include <lib/glconf.h>

#define SYNTAX_STR "Syntax %s [-d | -g /glftpd -s /site/] -o <file_id.diz> -f <file> [-a]\n"

typedef struct dirlog dirlog_t;

int create_rlsname(char *dirname, char *rlsname, char* grpname, int rlen, int glen) {
  char *t;

  t = strrchr(dirname, '/');
  if (t) {
    strncpy(rlsname, t + 1, rlen);
  }
  else {
    strncpy(rlsname, dirname, rlen);
  }


  // copy + strip group name
  t = strrchr(rlsname, '-');

  if (t) {
    strncpy(grpname, t + 1, glen);
    *t = 0;
  }
  else
    strcpy(grpname, "");

  t = rlsname;
  while (*t) {
    // replace '.', '_' with spaces
    if (*t == '.' || *t == '_')
      *t = ' ';
    t++;
  }

  return 0;
}

int prep_diz(FILE* output, char *grpname, char* diz_dir) {

  char* t;
  FILE* f;
  char buf[255];

  if (diz_dir == NULL)
    return 0;

  t = grpname;
  while (*t) {
    *t = tolower(*t);
    t++;
  }

  snprintf(buf, 255, "%s/%s.diz", diz_dir, grpname);

  f = fopen(buf, "r");

  if (f) {
    while (fgets(buf, 255, f) != NULL) {
      fprintf(output, "%s", buf);
    }
    fclose(f);
  }

  return 1;
}

int create_diz(FILE *output, char *adirname, int num_files, char *diz_dir) {

  char rlsname[120];
  char groupname[20];

  // build release name from the dirname
  create_rlsname(adirname, rlsname, groupname, 120, 20);

  free(adirname);

  if (num_files < 0) {
    fprintf(output, "%-37.37s [x/xx]\n", rlsname);
  }
  else {
    fprintf(output, "%-37.37s [x/%02d]\n", rlsname, num_files);
  }

  prep_diz(output, groupname, diz_dir);

  return 0;
}

int create_diz_from_dir(FILE* output, char* filename, char* diz_dir) {
  sfv_list_t *sfv = 0;
  char *adirname = 0;
  int num_files = -1;

  adirname = dirname(strdup(filename));

  // check if there is a .sfv in the dir, get num_files from it
  sfv = sfv_list_load_path(adirname);
  if (sfv != 0) {
    num_files = sfv_list_count(sfv);
    sfv_list_unload(sfv);
  }

  return create_diz(output, adirname, num_files, diz_dir);
}

int create_diz_from_log(FILE* output, char* filename, char* diz_dir, char* gl_home, char* site_prefix) {

  FILE* dl;
  dirlog_t dle;
  int rc, len;
  char buf[255];
  char* adirname;
  char* abasename;

  snprintf(buf, 255, "%s/ftp-data/logs/dirlog", gl_home);
  dl = fopen(buf, "rb");
  if (dl == 0) {
    fprintf(output, "isodiz-error(dirlog): %s\n", strerror(errno));
    return 201;
  }

  // adirname; /glftpd/site/blabla/file.txt -> /site/blabla/file.txt
  adirname = strstr(filename, site_prefix);

  if (adirname == 0) {
    fprintf(output, "isodiz-error(usage): site_prefix not found.\n");
    return 202;
  }

  // adirname; /site/blabla/file.txt -> /site/blabla
  adirname = dirname(strdup(adirname));

  len = sizeof(dirlog_t);

  while (1) {

    rc = fread(&dle, len, 1, dl);

    if (rc <= 0)
      break;

    if (strcasecmp(dle.dirname, adirname) == 0) {
      break;
    }
  }
  fclose(dl);

  if (rc <= 0) {
    create_diz(output, adirname, -1, diz_dir);
  }
  else {
    create_diz(output, adirname, dle.files, diz_dir);
  }

}

int create_diz_in_sfv(char *sfvfile, char *output_file) {
  sfv_list_t *sfv, *t;
  FILE *sh, *diz;
  char buf[1024];
  
  // load sfv
  sfv = sfv_list_load(sfvfile);

  if (sfv == NULL)
	return -1;

  diz = fopen(output_file, "r");

  if (diz == NULL)
	return -1;

  sh = fopen(sfvfile, "w");

  if (sh == NULL)
	return -1;

  fprintf(sh, "; @BEGIN_FILE_ID.DIZ\n");
  while (fgets(buf, 1000, diz) != NULL) {
	fprintf(sh, "; %s", buf);
  }
  fprintf(sh, "; @END_FILE_ID.DIZ\n");
  for (t = sfv; t != NULL; t = t->next) {
	fprintf(sh, "%s %lx\n", t->filename, t->crc);
  }

  sfv_list_unload(sfv);

  fprintf(sh, "; isodiz (c) tanesha\n");

  fclose(diz);
  fflush(sh);
  fclose(sh);

  return 0;
}


int main(int argc, char* argv[]) {

  char* gl_home = "/glftpd";
  char* site_prefix = "/site/";
  char* check_file = 0;
  char* output_file = "-";
  FILE* output;
  char* diz_dir = 0;
  int use_dir = 0;
  int diz_to_sfv = 0;
  int opt;

  while ((opt = getopt(argc, argv, "adg:f:o:z:s:")) != -1) {
    switch (opt) {
	case 'a':
	  diz_to_sfv = 1;
	  break;
	case 's':
	  site_prefix = strdup(optarg);
	  break;
    case 'd':
      use_dir = 1;
      break;
    case 'g':
      gl_home = strdup(optarg);
      break;
    case 'o':
      output_file = strdup(optarg);
      break;
    case 'f':
      check_file = strdup(optarg);
      break;
    case 'z':
      diz_dir = strdup(optarg);
      break;
    default:
      printf(SYNTAX_STR, argv[0]);
      exit(101);
    }
  }

  if (output_file == 0 || check_file == 0 || (gl_home == 0 && use_dir == 0)) {
    printf(SYNTAX_STR, argv[0]);
    exit(102);
  }

  if (strcmp(output_file, "-") == 0) {
    output = stdout;
  }
  else {
    output = fopen(output_file, "w");
  }

  if (use_dir > 0)
    create_diz_from_dir(output, check_file, diz_dir);
  else
    create_diz_from_log(output, check_file, diz_dir, gl_home, site_prefix);

  fflush(output);
  fclose(output);

  if (diz_to_sfv && (strstr(check_file, ".sfv") != NULL)) {
	create_diz_in_sfv(check_file, output_file);
  }
}
