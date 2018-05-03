/*
 * foo-tools, a collection of utilities for glftpd users.
 * Copyright (C) 2003  Tanesha FTPD Project, www.tanesha.net
 *
 * This file is part of foo-tools.
 *
 * foo-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * foo-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with foo-tools; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "spy_view.h"
#include "spy.h"

#include <curses.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>
#include <lib/dirlist.h>

// the windows.
WINDOW *wuserlist;
WINDOW *wuserinfo;
WINDOW *wstatus;

#define USER_RENDER " %3d  %-12.12s %-8.8s %s"
#define PID_MIN 100

// calcd version for userlist size since it depends on term size.
int userlist_size = -1;
int w_x = -1;
spy_status_t status;

void spy_view_refresh() {
	wnoutrefresh(wuserlist);
	wnoutrefresh(wuserinfo);
	wnoutrefresh(wstatus);
	doupdate();
}

void spy_view_deinit() {
	endwin();
}

void spy_makeage(time_t t, time_t age, char *buf) {
        time_t days=0,hours=0,mins=0,secs=0;
        
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

/*
 * Creates the windows for other routines to use.
 */
void spy_view_init(int max) {
	int x, y;

	status.max = max;

	// get main window.
	initscr();

	// some curses init.
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);

	// get window size.
	getmaxyx(stdscr, y, x);
	w_x = x - 2;
	
	// draw box around.
	box(stdscr, ACS_VLINE, ACS_HLINE);
	mvwhline(stdscr, 1 + STATUS_SIZE , 1, ACS_HLINE, w_x);
	mvwhline(stdscr, y - 2 - USERINFO_SIZE, 1, ACS_HLINE, w_x);

	// draw main window.
	refresh();

	// status window
	wstatus = subwin(stdscr, STATUS_SIZE, w_x, 1, 1);

	// userlist window
	userlist_size = y - STATUS_SIZE - USERINFO_SIZE - 4;
	wuserlist = subwin(stdscr, userlist_size, w_x, STATUS_SIZE + 2, 1);

	// userinfo window
	wuserinfo = subwin(stdscr, USERINFO_SIZE, w_x, y - 1 - USERINFO_SIZE, 1);
}


void spy_view_userlist_render_user(time_t now, spy_list_t *user, char *buf) {
	struct ONLINE *o;
	int rc;
	char tbuf[300], tfile[300], *last, age[50];
	double speed;

	if (!user)
		return;

	o = user->user;

	if (o->procid == 0) {
		sprintf(buf, USER_RENDER, user->pos, "", "", "");
		return;
	}

	status.online++;
	spy_makeage(o->tstart.tv_sec, now, age);
	
	sprintf(tbuf, o->status);
	last = (char*)&tbuf;
	while (*last)
		if ((*last == '\r') || (*last == '\n'))
			*last = 0;
		else
			last++;

	sprintf(buf, USER_RENDER, user->pos, o->username, age, tbuf);

	rc = who_transfer_direction(o);

	if (rc == TRANS_NONE)
		return;

	who_transfer_file(o, tfile);
	speed = who_transfer_speed(o);
	last = strrchr(tfile, '/');
	if (last)
		last++;
	else
		last = (char*)&tfile;

	if (rc == TRANS_UP) {
		status.ul_speed += speed;
		status.ul_num++;
	}
	else if (rc == TRANS_DOWN) {
		status.dl_speed += speed;
		status.dl_num++;
	}

	sprintf(tbuf, "%s: %s %5dk %.1fks", (rc == TRANS_UP)?"UL":"DL", last, o->bytes_xfer/1024, speed);

	sprintf(buf, USER_RENDER, user->pos, o->username, age, tbuf);
}

void spy_view_fit_buf(WINDOW *w, char *fmt, ...) {
	char b[300];
	va_list va;

	va_start(va, fmt);
	vsprintf(b, fmt, va);
	va_end(va);

	if (strlen(b) > w_x)
		b[w_x] = 0;
	else if (strlen(b) < w_x) {
		while (strlen(b) < w_x)
			strcat(b, " ");
	}

	wprintw(w, b);
}


void spy_view_userlist_render(spy_list_t *all, spy_list_t *top, spy_list_t *selected) {
	int i = 0, view = 0;
	spy_list_t *tmp;
	char buf[300];
	time_t now;

	tmp = all;
	now = time(0);

	wmove(wuserlist, 0, 0);

	while (tmp) {
		spy_view_userlist_render_user(now, tmp, buf);

		if (tmp == top)
			view++;

		if ((status.mode == MODE_USERLIST) && view && (i < userlist_size)) {

			wmove(wuserlist, i, 0);
			if (tmp == selected)
				wattron(wuserlist, A_REVERSE);
			spy_view_fit_buf(wuserlist, "%s", buf);
			if (tmp == selected)
				wattroff(wuserlist, A_REVERSE);

			i++;
		}

		tmp = tmp->next;
	}
}

void spy_view_dirlist_render(spy_list_t *cu) {
	int i;
	dirlist_t dl;
	dirlist_item_t *di;
	char *tmp, buf[300], obuf[300];
	struct stat st;

	buf[0] = 0;

	if (status.mode != MODE_DIRLIST)
		return;


	if (cu->user->procid == 0) {
		wmove(wuserlist, 0, 0);
		spy_view_fit_buf(wuserlist, "No user online on selected node, press 'u' to get to userlist");

		return;
	}

	sprintf(buf, "%s%s", status.glroot, cu->user->currentdir);
	if (stat(buf, &st) == -1)
		return;

	if (!S_ISDIR(st.st_mode)) {
		tmp = strrchr(buf, '/');

		if (!tmp)
			return;

		*tmp = 0;
	}	

	wmove(wuserlist, 0, 0);

	wprintw(wuserlist, "Dir: %s\n", buf);

	dirlist_init(&dl, buf);

	if (!dirlist_readdir(&dl)) {
		wprintw(wuserlist, "Cannot open dir :(\n");

		return;
	}

	dirlist_reset(&dl);

	i = 2;
	while ((i < userlist_size) && (di = dirlist_next(&dl))) {
		sprintf(obuf, " %-32.32s(%2d) |", di->file, di->downloads);
		
		di = dirlist_next(&dl);
		if (di) {
			sprintf(buf, " %-32.32s(%2d) ", di->file, di->downloads);
			strcat(obuf, buf);
		}

		wmove(wuserlist, i, 0);
		spy_view_fit_buf(wuserlist, obuf);
		i++;
	}

	dirlist_closedir(&dl);
}

void spy_view_status_render() {

	wmove(wstatus, 0, 0);

	spy_view_fit_buf(wstatus, "Online  : %3d of %3d  -  foo-spy (c) tanesha team", status.online, status.max);
	wmove(wstatus, 1, 0);
	spy_view_fit_buf(wstatus, "Upload  : %3d at%10.1fKbs (avg: %7.1fKbs, max: %.1fKbs)",
			status.ul_num, status.ul_speed,
			status.ul_speed/status.ul_num,
			status.max_ul_speed);
	wmove(wstatus, 2, 0);
	spy_view_fit_buf(wstatus, "Download: %3d at%10.1fKbs (avg: %7.1fKbs, max: %.1fKbs)",
		status.dl_num, status.dl_speed, status.dl_speed/status.dl_num,
		status.max_dl_speed);
	wmove(wstatus, 3, 0);
	spy_view_fit_buf(wstatus, "Total   : %3d at%10.1fKbs (avg: %7.1fKbs, max: %.1fKbs)",
		status.dl_num + status.ul_num,
		status.dl_speed + status.ul_speed,
		(status.dl_speed + status.ul_speed) / (status.dl_num + status.ul_num),
		(float)0);

}

void spy_view_userinfo_render(spy_list_t *sel) {
	struct ONLINE *o;
	char buf[60];

	o = sel->user;

	if (o->procid == 0)
		return;

	spy_makeage(o->login_time, time(0), buf);

	wmove(wuserinfo, 0, 0);
	wprintw(wuserinfo, "Unfo: %-55.55s Pid: %5d\n", o->tagline, o->procid);
	wprintw(wuserinfo, "Cwd : %-55.55s Gid: %5d\n", o->currentdir, o->groupid);
	wprintw(wuserinfo, "Host: %-55.55s Onl: %-6.6s", o->host, buf);
}

int spy_list_diff(spy_list_t *t, spy_list_t *s) {
	spy_list_t *tmp;
	int c = 0;

	tmp = t;
	while (tmp != s) {
		tmp = tmp->next;
		c++;
	}

	return c;
}

void spy_view_handler(spy_list_t *who) {
	int i = 0, key, distance;
	spy_list_t *top, *selected;

	top = selected = who;
	timeout(500);
	status.mode = MODE_USERLIST;

	while (1) {
		// check for keypress.
		key = getch();

		if (key == KEY_UP) {
			if (selected && selected->prev)
				if (selected == top)
					top = selected = selected->prev;
				else
					selected = selected->prev;
		}
		else if (key == KEY_DOWN) {
			if (selected && selected->next) {

				// move the top pointer down if we're about to scroll.
				distance = spy_list_diff(top, selected);
				if (distance == userlist_size-1)
					top = top->next;

				selected = selected->next;
			}
		}
		else if (toupper(key) == 'Q')
			break;
		else if (toupper(key) == 'K') {
			// safeguard against fucked up pid when running as root.
			if (selected->user->procid > PID_MIN)
				kill(selected->user->procid, SIGTERM);
		}
		else if (toupper(key) == 'D') {
			status.mode = MODE_DIRLIST;
			wclear(wuserlist);
		}
		else if (toupper(key) == 'U') {
			status.mode = MODE_USERLIST;
			wclear(wuserlist);
		}

		// reset the stats.
		status.ul_num = status.dl_num = 0;
		status.ul_speed = status.dl_speed = 0;
		status.online = 0;

		spy_view_userlist_render(who, top, selected);

		if (status.ul_speed > status.max_ul_speed)
			status.max_ul_speed = status.ul_speed;
		if (status.dl_speed > status.max_dl_speed)
			status.max_dl_speed = status.dl_speed;

		spy_view_dirlist_render(selected);
		spy_view_status_render();
		spy_view_userinfo_render(selected);

		spy_view_refresh();

		i++;
	}
	
}
