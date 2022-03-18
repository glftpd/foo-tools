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

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../foo-pre.h"
#include <collection/hashtable.h>
#include <lib/gllogs.h>

// test a module.
int test_module(char *module, char *file, char *path) {

	void *handle;
	module_list_t *module_func;
	module_list_t* (*module_loader)();
	char *args[] = {"", "Whatever-REL", "mp3"};

	handle = dlopen(module, RTLD_LAZY);

	if (!handle) {
		printf("Error loading module '%s':\n%s\n", module, dlerror());
		return 0;
	}

	module_loader = dlsym(handle, MODULE_LOADER_FUNC);

	if (!module_loader) {
		printf("Error loading module %s: No loader func\n");
		dlclose(handle);
		return 0;
	}

	printf("DEBUG: file=%s path=%s\n", file, path);

	// show module name
	printf("module name = %s %s\n", module_loader()->mod_name, module);

/* FIXME
	// test module file func
	if (module_func->mod_func_file != 0)
		module_loader()->mod_func_file(file, args);

	// test module dir func
	if (module_func->mod_func_dir != 0)
		module_loader()->mod_func_dir(path, args);

	// test module rel func
	if (module_func->mod_func_rel != 0)
		module_loader()->mod_func_rel(path, args);
*/

	dlclose(handle);

	return 1;
}

hashtable_t *cfg = 0;
hashtable_t *get_config() {
	return cfg;
}

/* FIXME
int do_module(char *path, char *file) {

	//void *handle;
	module_list_t *module_func;
	//module_list_t* (*module_loader)();
	char *args[] = {"", "Whatever-REL", "mp3"};

	module_func = module_loader();

        //module_loader = dlsym(handle, MODULE_LOADER_FUNC);

	if (module_func->mod_func_file != 0)
		module_func->mod_func_file(file, args);

	if (module_func->mod_func_dir != 0)
		module_func->mod_func_dir(path, args);

	if (module_func->mod_func_rel != 0)
		module_func->mod_func_rel(path, args);
}
*/

int main(int argc, char *argv[]) {

	if (!argv[2]) {
		printf("Error no module specified\n");
		exit(1);
	}

	cfg = malloc(sizeof(hashtable_t));
	ht_init(cfg);
	ht_load(cfg, argv[1]);

	get_config();
	//gl_gllog_add("");

	test_module(argv[2], argv[3], argv[4]);

	//FIXME
	//do_module(argv[2], argv[3]);

}
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
