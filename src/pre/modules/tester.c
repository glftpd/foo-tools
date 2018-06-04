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
#include <malloc.h>
#include <stdlib.h>

#include "../foo-pre.h"
#include <collection/hashtable.h>

// to test (un)comment one of these 2 functions in main below:
//   do_module   -> to run: ./tester <cfg> <path> <file>
//   test_module -> to run: ./tester mod_name.so <file> <path>

// set to "1" to run test or "0" to skip:
int test_file = 0;
int test_dir = 1; 	

// test a module.

int test_module(char *module, char *file, char *path) {
/*
        //hashtable_t *_config = 0;
        hashtable_t *_envctx = 0;

        hashtable_t * get_context() {
                if (!_envctx) {
                        _envctx = malloc(sizeof(hashtable_t));
                       ht_init(_envctx);
                }

                return _envctx;
        }
        hashtable_t *env = get_context(); 
        ht_put(env,"section","mp3");
*/

        setenv("section","mp3",1);
        printf("section: %s\n", getenv("section"));

	void *handle;
	module_list_t *module_func;
	module_list_t* (*module_loader)();
	char *args[] = {"", "Whatever-REL", "mp3"};


        handle = dlopen(module, RTLD_LAZY);
	if (!handle) {
		printf("Error loading module %s: %s\n", module, dlerror());
		return 0;
	}

	module_loader = dlsym(handle, MODULE_LOADER_FUNC);

	if (!module_loader) {
		printf("Error loading module %s: No loader func\n");
		dlclose(handle);
		return 0;
	}

	// show module name
	printf("module name = %s\n", module_loader()->mod_name);

	printf("test file: %i dir: %i\n", test_file, test_dir);

	// test module file func
        if (test_file) {
		printf("test module file func - file: %s args0: %s args1: %s args2: %s\n", file, args[0], args[1], args[2]);
		module_loader()->mod_func_file(file, args);
	}

	// test module dir func
        if (test_dir) {
		printf("test module dir func - path: %s args0: %s args1: %s args2: %s\n", path, args[0], args[1], args[2]);
		module_loader()->mod_func_dir(path, args);
	}

	dlclose(handle);

	return 1;
}
hashtable_t *cfg = 0;
hashtable_t *get_config() {
	return cfg;
}

int do_module(char *path, char *file) {
        printf("DEBUG: do_module");
	module_list_t *module_func;
        module_list_t* (*module_loader)();
	char *args[] = {"", "Whatever-REL", "mp3"};

	module_func = module_loader();

        if (test_file) {
		if (module_func->mod_func_file)
		        printf("test module file func - file: %s args0: %s args1: %s args2: %s\n", file, args[0], args[1], args[2]);
			module_func->mod_func_file(file, args);
	}

        if (test_dir) {
		if (module_func->mod_func_dir)
		        printf("test module dir func - path: %s args0: %s args1: %s args2: %s\n", path, args[0], args[1], args[2]);
			module_func->mod_func_dir(path, args);
	}
}

int main(int argc, char *argv[]) {
        if (argc < 2) {
                printf("missing args\n");
                exit(0);
        }
	cfg = malloc(sizeof(hashtable_t));
	ht_init(cfg);
	//ht_load(cfg, argv[1]);
	ht_load(cfg, "testpre.cfg");
        char buf[1024];
        sprintf(buf, "section.%s.%s", "mp3", PROPERTY_SECTION_DIR);
        printf("config - modules: %s\n", ht_get(get_config(), PROPERTY_MODULES));
        printf("config - buf: %s section: %s\n", buf, ht_get(get_config(), buf));

	get_config();
	//gl_gllog_add("");
	test_module(argv[1], argv[2], argv[3]);

	//do_module(argv[2], argv[3]);

}
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
