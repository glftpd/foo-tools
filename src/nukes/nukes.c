/*
 * Tanesha Team! $Id: nukes.c,v 1.10 2002/03/15 23:29:05 flower Exp $
 */

#include "glnukelog.h"
#include <collection/hashtable.h>
#include "nukes.h"
#include <lib/macro.h>
#include <collection/strlist.h>

hashtable_t * _cfg = 0, *_ctx = 0;
time_t startat = 0;

hashtable_t * get_config() {
	if (_cfg == 0) {
		_cfg =malloc(sizeof(hashtable_t));
		ht_init(_cfg);
	}
	return _cfg;
}
hashtable_t * get_context() {
	if (_ctx == 0) {
		_ctx =malloc(sizeof(hashtable_t));
		ht_init(_ctx);
	}
	return _ctx;
}

//             123456789012|1234567890123456789012345678901234567890123|1234567890123456
//              [ 10  x1 ] | /PC-Games


void nukes_makeage(time_t t, char *buf) {
        time_t days=0,hours=0,mins=0,secs=0, age;
		
		if (startat == 0)
			startat = time(0);

		age = startat;

        if (t>=age)
                sprintf(buf,"0m 0s");
        else {
                age-=t;
                days=age/(3600*24);
                age-=days*3600*24;
                hours=age/3600;
                age-=hours*3600;
                mins=age/60;
                secs=age-(mins*60);

                if (days)
                        sprintf(buf,"%dd %dh",days,hours);
                else if (hours)
                        sprintf(buf,"%dh %dm",hours,mins);
                else
                        sprintf(buf,"%dm %ds",mins,secs);
        }
}


void nukes_fit_path(char *dir, char *o, int len) {
	if (strlen(dir) <= len)
		strcpy(o, dir);
	else {
		strcpy(o, dir + strlen(dir) - len);
		o[0] = '.';
		o[1] = '.';
	}
}

char *nukes_reason_next(char *p, char *b, int len) {
	if (strlen(p) <= len)
		strcpy(b, p);
	else {
		strncpy(b, p, len);
		b[len] = 0;
	}

	return p + strlen(b);
}


void nukes_print_single(nukelog_t *n, int pos, char *body) {
	char *reason, smult[50], buf[500], rbuf[300];
	int line = 0;
	nukeelog_t *nl;

	reason = n->reason;
	sprintf(smult, "x%d", n->mult);

	nukes_fit_path(n->dirname + strlen(NUKES_SITEROOT), buf, 41);
	reason = nukes_reason_next(reason, rbuf, 15);

	// print first line.
	printf("<  %2d   %3.3s > %-41.41s | %-15.15s\n", pos, smult, buf, rbuf);

	sortedlist_reset(&n->nukees);

	// print rest lines.
	while ((line < 2) || (strlen(reason) > 0) || sortedlist_hasnext(&n->nukees)) {
		if (line == 0)
			sprintf(smult, "%s %c", n->nuker, n->status==0?'N':'U');
		else if (line == 1)
			nukes_makeage(n->nuketime, smult);
		else
			smult[0] = 0;

		nl = sortedlist_next(&n->nukees);

		if (nl) {
			sprintf(buf, "%s - %.1f Mb", nl->nukee, nl->bytes);
			if (!sortedlist_hasnext(&n->nukees)) {
				sprintf(rbuf, "total: %.1f Mb", n->bytes);
				sprintf(buf, "%-20.20s%20.20s", buf, rbuf);
			}
		} else
			sprintf(buf, "");
		
		reason = nukes_reason_next(reason, rbuf, 15);

		printf("%11.11s |  %-40.40s | %-15.15s\n", smult, buf, rbuf);

		line++;
	}

	printf(DELIM);
}


int nukes_match_nukee(sortedlist_t *l, char *u) {
	int found = 0;
	nukeelog_t *n;

	sortedlist_reset(l);

	while (sortedlist_hasnext(l)) {
		n = sortedlist_next(l);

		if (!strcasecmp(n->nukee, u)) {
			found = 1;
			break;
		}
	}

	return found;
}


char* nukes_strcasestr(char *h, char *n) {
	char *hh, *nn, *tmp;

	hh = strdup(h);
	nn = strdup(n);

	for (tmp = hh; *tmp; tmp++)
		*tmp = tolower(*tmp);
	for (tmp = nn; *tmp; tmp++)
		*tmp = tolower(*tmp);

	return strstr(hh, nn);
}

int nukes_match(nukelog_t *t) {
	int i = 0, tmpi;
	char buf[20], *tmp;
	hashtable_t *ctx = get_context();
	int match = 1;

	while (1) {
		sprintf(buf, "s.%d", i++);
		tmp = ht_get(ctx, buf);

		if (!tmp)
			break;

		if (!strncasecmp(tmp, "nuker=", 6)) {
			if (strcasecmp(t->nuker, tmp + 6)) {
				match = 0;
				break;
			}
		}
		else if (!strncasecmp(tmp, "nukee=", 6)) {
			if (!nukes_match_nukee(&t->nukees, tmp + 6)) {
				match = 0;
				break;
			}
		}
		else if (!strncasecmp(tmp, "status=", 7)) {
			switch (*(tmp + 7)) {
				case 'n':
				case 'N': if (t->status != 0) match = 0;
					break;
				case 'U':
				case 'u': if (t->status != 1) match = 0;
					break;
				default: match = 1;
			}
			if (match == 0)
				break;
		}
		else if (!strncasecmp(tmp, "reason=", 7)) {
			if (!nukes_strcasestr(t->reason, tmp + 7)) {
				match = 0;
				break;
			}
		}
		else if (!strncasecmp(tmp, "factor=", 7)) {
			tmpi = atoi(tmp + 7);
			if (t->mult != tmpi) {
				match = 0;
				break;
			}
		}
		else if (strcasecmp(t->nuker, tmp) &&
				 !nukes_match_nukee(&t->nukees, tmp) &&
				 !nukes_strcasestr(t->reason, tmp) &&
				 !nukes_strcasestr(t->dirname, tmp)) {
			match = 0;
			break;
		}
	}

	return match;
}


int nukes_check() {
	sortedlist_t list;
	nukelog_t *t;
	int i = 0, show = 10;
	char *tmp, *singleshow;
	hashtable_t *cfg, *ctx;

	ctx = get_context();
	cfg = get_config();

	printf(HEAD);

	sortedlist_init(&list);

	nukelog_load(&list, NUKES_NUKELOG);

	sortedlist_reset(&list);

	tmp = ht_get(ctx, "show");
	if (tmp)
		show = atoi(tmp);

	while ((i < show) && (t = (nukelog_t*)sortedlist_next(&list))) {

		// check if matches criteria.
		if (!nukes_match(t))
			continue;

		i++;

		nukes_print_single(t, i, singleshow);
	}

	printf(TAIL);
}

int _nukes_numeric(char *t) {
	char *num = "1234567890";
	int i, isnum = 1;

	for (i = 0; i < strlen(t); i++)
		if (!strchr(num, t[i]))
			isnum = 0;

	if (isnum > 0)
		return atoi(t);

	return -1;
}


int nukes_init(int argc, char *argv[]) {
	hashtable_t *t;
	int i, tmp, j = 0;
	char buf[20];

	t = get_config();
	// ht_load_prop(t, NUKES_CONFIGFILE, '=');

	t = get_context();

	for (i = 1; i < argc; i++) {
		tmp = _nukes_numeric(argv[i]);

		if (tmp > 0) {
			// set maxshow
			ht_put(t, "show", argv[i]);
		}
		else {
			// add searchword
			sprintf(buf, "s.%d", j++);
			ht_put(t, buf, argv[i]);
		}
	}
}

void nukes_help() {
  printf("
Syntax; site nukes <arg1> <arg2> .. <argn>

Arguments can be:
  Any word  -  Will search reason/nuker/dirname/nukee for the word.
  Any number-  Will make output list <number> newest found nukes.
  nuker=<n> -  Will search for items where nuker is <n>.
  nukee=<n> -  Will search for items where nukee is <n>.
  reason=<r>-  Will search for items where reason contains <r>.
  status=<s>-  Will search for statuses, where <s> is nuked or unnuked.
  factor=<f>-  Will search for nukes with factor <f>.

Examples:
  site nukes 10 nuker=flower       - List 10 latest nukes by flower
  site nukes 3 -FLT                - List 3 latest -FLT nukes.
  site nukes reason=size games     - List where reason contains size
                                     and 'games' is in the nuke.
");

  exit(0);
}

/*
 * Main test routine.
 */
int main(int argc, char *argv[]) {

  if ((argc > 1) && !strcasecmp(argv[1], "help"))
    nukes_help();

	nukes_init(argc, argv);

	nukes_check();

	return 0;
}

