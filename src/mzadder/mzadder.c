/* muggi zip-adder v1.1 *FINAL* (c) flower/prjdd'98				

   feel free to mod source, but dont forget to mail me the new one.	*/

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <ddlib.h>
#include <dd.h>
#include <sys/stat.h>
#include <zip.h>
#include <string.h>

#include <lib/macro.h>
#include <collection/hashtable.h>
#include <collection/strlist.h>
#include <util/strmatch.h>
#include <lib/common.h>
#include <lib/stringtokenizer.h>

struct daydream_callback {
  struct dif *dd;
  char *data;
  void (*sendstring)(struct dif *d, const char *str);
  int (*changestatus)(struct dif *d, char *status);
  void (*getstrval)(struct dif *d, char *str, int i);
  void (*close)(struct dif *d);
};

void test_close(struct dif *d) {
  printf("! closing ..\n");
}
void test_sendstring(struct dif *d, const char *str) {
  printf("> %s\n", str);
}

int test_changestatus(struct dif *d, char *status) {
  printf("! Status %s\n", status);
}

void test_getstrval(struct dif *d, char *str, int i) {

  printf("! getstrval %d\n", i);

  switch (i) {
  case 103:
	strcpy(str, "tester");
	break;
  case 104:
	strcpy(str, "ok land");
	break;
  case 129:
	strcpy(str, (char*) d);
	break;
  default:
	strcpy(str, "OK");
  }
}

int ht_left_trim_values(hashtable_t *ht) {
  hashtable_item_t *i;
  char *t, *n;

  for (i = ht->list; i != NULL; i = i->next) {
	t = i->value;

	// trim initial spaces or tabs
	while (*t == ' ' || *t == '\t') {
	  t++;
	}

	n = strdup(t);
	free(i->value);
	i->value = n;
  }
}

void initrand() {
  time_t t;
  struct tm *tid;

  time(&t);
  tid=localtime(&t);
  srandom((tid->tm_min)*(tid->tm_sec));
}

int frandom(int modseed) {
  return ((random()>>3)%modseed);
}

int zipad_filename(char *pattern, char *out) {
  char *t;
  
  strcpy(out, pattern);
  t = out;

  while (*t) {
	switch(*t) {
	case '$' :	
	  *t=(char)(frandom(10)+48);
	  break;
	case '@' :
	  *t=(char)(frandom(25)+97);
	  break;
	case '?' :
	  if ((frandom(2)))
		*t=(char)(frandom(25)+97);
	  else
		*t=(char)(frandom(10)+48);
	};
	t++;
  }

  return 1;
}

float getfs(char *node,char *fname) {
  char tmp[255];
  struct stat st;

/*  sprintf(tmp,"%s%s/%s",temppath,node,fname); */
  stat(fname,(struct stat*)&st);
  return st.st_size / (1024 * 1024);
};

int macro_get(struct macro_list *ml, char *key, char *value) {
  struct macro_list *t;

  for (t = ml; t != NULL; t = t->next) {
	if (strcasecmp(t->mac_key, key) == 0) {
	  strcpy(value, t->mac_rep);
	  return 1;
	}
  }

  return 0;
}

struct macro_list *build_macros(struct daydream_callback *d, char *node) {
  struct macro_list *ml = NULL;
  time_t t;
  struct tm *tid;
  char buf[500];

  time(&t);
  tid=localtime(&t);

  // DATE
  sprintf(buf,"%4.4d-%2.2d-%2.2d", tid->tm_year + 1900, tid->tm_mon + 1, tid->tm_mday);
  ml = ml_addstring(ml, "DATE", buf);

  // TIME
  sprintf(buf, "%2.2d:%2.2d:%2.2d", tid->tm_hour, tid->tm_min, tid->tm_sec);
  ml = ml_addstring(ml, "TIME", buf);

  // STIM
  sprintf(buf, "%2.2d:%2.2d", tid->tm_hour, tid->tm_min);
  ml = ml_addstring(ml, "STIM", buf);

  // node
  ml = ml_addstring(ml, "NODE", node);

  // user handle
  d->getstrval(d->dd, buf, USER_HANDLE);
  ml = ml_addstring(ml, "UN", buf);

  d->getstrval(d->dd, buf, USER_ORGANIZATION);
  ml = ml_addstring(ml, "UL", buf);
  
  d->getstrval(d->dd, buf, DOOR_PARAMS);
  ml = ml_addstring(ml, "FN", buf);

  ml = ml_addfloat(ml, "FS", getfs(node, buf));
  
  return ml;
}

void dd_send_buf(struct dif *d, char *text) {

  dd_sendstring(d, text);

  /*
  char *t, *o, *l;

  l = o = t = strdup(text);

  while (*t) {

	if (*t == '\n' || *t == '\r') {
	  *t = 0;
	  dd_sendstring(d, l);

	  if (*t == '\r' && *(t + 1) == '\n')
		t += 2;
	  else
		t += 1;

	  l = t;
	}
	else {
	  t++;
	}
  }

  if (t != l)
	dd_sendstring(d, l);

  free(o);
  */
}

int build_random_csv(char *str, char *selected) {
  stringtokenizer st;
  int len, rand, i;
  char *r;

  st_initialize(&st, str, ",");
  len = st_count(&st);
  st_reset(&st);

  if (len == 0)
	return 0;

  rand = frandom(len);

  for (i = 1; i < rand; i++)
	st_next(&st);

  strcpy(selected, st_next(&st));

  st_finalize(&st);

  return 1;
}

int zip_update(struct zip *zip, char *filename, char *content) {
  struct zip_source *zcontent;
  int idx;

  if (zip == NULL)
	return -1;

  zcontent = zip_source_buffer(zip, content, strlen(content), 1);

  idx = zip_name_locate(zip, filename, ZIP_FL_NOCASE);

  if (idx == -1) {
	idx = zip_add(zip, filename, zcontent);
  }
  else {
	idx = zip_replace(zip, idx, zcontent);
  }

  // zip_source_free(zcontent);
}

int process_zipad(hashtable_t *cfg, struct macro_list *ml, char *fn, struct zip *zip) {

  char *tmp, *file;
  char outfn[255];
  char zipadfile[255];

  tmp = ht_get(cfg, "zipad.pattern");
  if (tmp)
	zipad_filename(tmp, outfn);
  else
	strcpy(outfn, "zipad.nfo");
  
  tmp = ht_get(cfg, "zipad.files");
  
  build_random_csv(tmp, zipadfile);
  
  file = readfile(zipadfile);

  // ad empty file.
  if (file == NULL)
	tmp = strdup("");
  else
	tmp = ml_replacebuf(ml, file);
  
  // update the zip structure
  zip_update(zip, outfn, tmp);
}

strlist_t *tokenize_to_list(char *str, char *delim) {
  
  stringtokenizer st;
  strlist_t *l = NULL;

  if (str == NULL)
	return l;

  st_initialize(&st, str, delim);
  st_reset(&st);

  while (st_hasnext(&st)) {
	l = str_add_last(l, st_next(&st));
  }

  st_finalize(&st);

  return l;
}

char *str_get(strlist_t *list, int idx) {
  strlist_t *t;
  int i = 0;

  for (t = list; i < idx; t = t->next);

  return t->data;
}

int process_dizad(hashtable_t *cfg, struct macro_list *ml, char *fn, struct zip *zip) {

  char *dizad;
  strlist_t *ads = NULL, *diz = NULL;
  int rand;
  char buf[100];

  diz = str_load(diz, "./.packtmp/file_id.diz");

  // no diz
  if (diz == NULL)
	return -1;

  // find ads
  ads = tokenize_to_list(ht_get(cfg, "dizad.lines"), ",");

  // select random
  rand = frandom(str_count(ads));
  dizad = str_get(ads, rand);
  snprintf(buf, 100, "dizad.line.%s", dizad);

  dizad = ht_get(cfg, buf);

  // not found ? :(
  if (dizad == NULL)
	return -1;

  // expand macros
  dizad = ml_replacebuf(ml, dizad);

  // append ad to diz.
  diz = str_add_last(diz, dizad);
  dizad = str_join(diz, "\n");

  // update file_id in archive with new diz
  zip_update(zip, "file_id.diz", dizad);
}

int process_adremove(hashtable_t *cfg, struct macro_list *ml, char *fn, struct zip *zip) {
  
  char *pat;
  strlist_t *pl = NULL;
  strlist_t *del = NULL;
  strlist_iterator_t *it;
  int i, l;
  char nbuf[10];
  struct zip_stat st;

  pl = tokenize_to_list(ht_get(cfg, "adremove.patterns"), ",");

  if (pl == NULL)
	return -1;

  l = zip_get_num_files(zip);

  if (l == -1)
	return -1;

  for (i = 0; i < l; i++) {
	if (zip_stat_index(zip, i, 0, &st) != 0)
	  continue;

	it = str_iterator(pl);

	while (str_iterator_hasnext(it)) {
	  pat = str_iterator_next(it);

	  if (strmatch_filename(pat, st.name, STRMATCH_IGNORECASE)) {
		snprintf(nbuf, 10, "%d", i);
		del = str_add_last(del, nbuf);
	  }
	}
  }

  it = str_iterator(del);
  while (str_iterator_hasnext(it)) {
	pat = str_iterator_next(it);
	i = atoi(pat);
	zip_delete(zip, i);
  }
}

int process_zipcomment(hashtable_t *cfg, struct macro_list *ml, char *fn, struct zip *zip) {
  
  char *tmp, *file;
  char zipcommentfile[255];
  int len;

  tmp = ht_get(cfg, "zipcomment.files");
  
  if (tmp == NULL)
	return;

  // remove files
  if (strcasecmp(tmp, "remove") == 0) {
	zip_set_archive_comment(zip, NULL, 0);
	return 1;
  }

  build_random_csv(tmp, zipcommentfile);
  
  file = readfile(zipcommentfile);

  // ad empty file.
  if (file == NULL) {
	tmp = NULL;
	len = 0;
  }
  else {
	tmp = ml_replacebuf(ml, file);
	len = strlen(tmp);
  }
  
  // update the zip structure with new comment
  zip_set_archive_comment(zip, tmp, len);

  return 1;
}

int process(hashtable_t *cfg, struct daydream_callback *d, struct macro_list *ml, char *fn) {
  FILE *in,*out;
  char buf[1000];
  int r, ze, se;
  struct strlist *diz=NULL;
  char outfn[100], zipadfile[255];
  char *mode, *tmp, *file;
  struct zip *zip;

  mode = ht_get(cfg, "mode");

  if (mode == NULL)
	return -1;

  zip = zip_open(fn, 0, 0);

  if (zip == NULL)
	return -1;

  // ad remove
  if (strstr(mode, "adremove") != NULL) {

	d->changestatus(d->dd, "MuggiZA: Adremove..");

	tmp = ht_get(cfg, "adremove.message");
	if (tmp)
	  d->sendstring(d->dd, tmp);

	process_adremove(cfg, ml, fn, zip);
  }

  // zipad
  if (strstr(mode, "zipad") != NULL) {

	d->changestatus(d->dd, "MuggiZA: Zipad..");

	tmp = ht_get(cfg, "zipad.message");
	if (tmp)
	  d->sendstring(d->dd, tmp);

	process_zipad(cfg, ml, fn, zip);
  }

  if (strstr(mode, "dizad") != NULL) {

	d->changestatus(d->dd, "MuggiZA: Dizad..");

	tmp = ht_get(cfg, "dizad.message");
	if (tmp)
	  d->sendstring(d->dd, tmp);

	process_dizad(cfg, ml, fn, zip);
  }

  if (strstr(mode, "zipcomment") != NULL) {

	d->changestatus(d->dd, "MuggiZA: Zipcomment..");

	tmp = ht_get(cfg, "zipcomment.message");
	if (tmp)
	  d->sendstring(d->dd, tmp);

	process_zipcomment(cfg, ml, fn, zip);
  }

  if (zip_close(zip) == -1) {
	zip_error_get(zip, &ze, &se);
	zip_error_to_str(buf, 999, ze, se);
	printf("zip close failed: %s\n", buf);
  }

  return 0;
}



int main(int argc,char *argv[]) {
  struct dif *d;
  hashtable_t ht;
  struct macro_list *ml;
  char fn[255];
  struct daydream_callback cb;

  initrand();

  // test mode
  if (argc == 4 && strcasecmp(argv[1], "-t") == 0) {
	cb.dd = (struct dif *) argv[3];
	cb.changestatus = test_changestatus;
	cb.sendstring = test_sendstring;
	cb.getstrval = test_getstrval;
	cb.close = test_close;
  }
  else if (argc == 3) {
	cb.dd = dd_initdoor(argv[1]);
	cb.changestatus = dd_changestatus;
	cb.sendstring = dd_sendstring;
	cb.getstrval = dd_getstrval;
	cb.close = dd_close;
  }
  else {
	printf("what? you need .help\n");
	printf("bmzadder <node> <cfgfile> <- would be a good idea!\n");
	return 100;
  }
  
  cb.changestatus(cb.dd, "MuggiZA: Loading..");

  ml = build_macros(&cb, argv[1]);

  macro_get(ml, "FN", fn);
  
  // OK, this is for a .zip file, we want to do work.
  if (strstr(fn, ".zip")|| strstr(fn, "*.ZIP")) {

	// load config
	ht_init(&ht);
	ht_load(&ht, argv[2]);
	// hack for left trimmed values
	ht_left_trim_values(&ht);

	cb.sendstring(d, ht_get(&ht, "message"));

	process(&ht, &cb, ml, fn);
  }

  ht_finalize(&ht);
  ml_free(ml);

  cb.close(cb.dd);

  return 0;
}
