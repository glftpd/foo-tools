# foo-tools
[![build_footools](https://github.com/silv3rr/foo-tools/actions/workflows/build.yml/badge.svg)](https://github.com/silv3rr/foo-tools/actions/workflows/build.yml)

Besides the more recent updates, the rest of the code base is preserved here mainly for historical reasons.

This software comes without any claims regarding security or support about how to use it.

## Changes

This is a modified and updated version of foo-tools by Tanesha Team:

- modification to foo-pre; shows mp3 genre in PRE line (no module needed)
- added date/time in foo-pre.log
- added mod_audiosort, mod_prebw
- updated Makefiles, fixed compiler warnings
- updates/fixes from upstream [glftpd/foo-tools](https://github.com/glftpd/foo-tools)

For more details see [CHANGELOG](src/CHANGES).

### Mp3genre

An (optional) 'mp3genre' modification for foo-pre is included, which adds mp3 genre of release in PRE output. It works without a module and can be switched on/off in pre.cfg.

Check [README.mp3genre](src/pre/README.mp3genre) on details how to install/configure.

Example:
```
[PRE] [MP3] Aritist-Title-WEB-2021-GRP by GRP with 10 files (100MB) of Pop.
```

### Original version

The original foo-tools version by Tanesha is available from this branch: [foo-orig](https://github.com/silv3rr/foo-tools/tree/foo-orig)

It does include fixes, but not the mp3genre modification.

## Documentation

### Installation

1) First get this repo with `git clone`
2) Build all tools: `cd src && ./configure && make build`
3) See README for specific tool in [doc](doc) dir

### Detailed instructions ###

- [README.1st](README.1st) Original README file
- [src/README.1st](src/README.1st) Building instructions
- [doc/HOWTO](doc/HOWTO) Notes about compiling
- [doc](doc) Documentation on installing/running the programs

### Modules

Addons for foo-pre; modules add functions but are not build into foo-pre binary

- [src/pre/README.modules](src/pre/README.modules)
- [src/pre/modules/README.mod_prebw](src/pre/modules/README.mod_prebw)
- [src/pre/modules/README.mod_audiosort](src/pre/modules/README.mod_audiosort)
- [src/pre/modules/README.mod_chmod](src/pre/modules/README.mod_chmod)
- [src/pre/modules/README.mod_idmp3](src/pre/modules/README.mod_idmp3)
- [src/pre/modules/README.mod_nfohandler](src/pre/modules/README.mod_nfohandler)
- [src/pre/modules/README.mod_sitenfoadd](src/pre/modules/README.mod_sitenfoadd)
- [src/pre/modules/README.mod_symlink](src/pre/modules/README.mod_symlink)

