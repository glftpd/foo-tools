# foo-tools

## Updates

This is an updated version of foo-tools:

- updates/fixes from https://github.com/glftpd/foo-tools
- Makefiles updated
- foo-pre has mp3 genre in PRE line (no module needed)
- added date/time in foo-pre.log
- added mod_audiosort

For more details see [CHANGELOG](src/CHANGES)

## Documentation

- [README.1st](README.1st) Original README file
- [src/CHANGES](src/CHANGES) CHANGELOG
- [src/README.1st](src/README.1st) Building instructions
- [doc/HOWTO](doc/HOWTO) Notes about compiling
- [doc](doc) Documentation on installing/running the programs
- [README.mp3genre](src/pre/README.mp3genre) mp3genre specific README (install/configure)

### Mp3genre

Includes optional 'mp3genre' for foo-pre which adds mp3 genre in PRE output, without a module.

Example:
```
[PRE] [MP3] Aritist-Title-WEB-2021-GRP by GRP with 10 files (100MB) of Pop.
```

Check [README.mp3genre](src/pre/README.mp3genre) on details how to install/configure

### Modules

Addons for foo-pre; modules add functions but are not build into foo-pre binary

- [src/pre/README.modules](src/pre/README.modules)
- [src/pre/modules/README.mod_audiosort](src/pre/modules/README.mod_audiosort)
- [src/pre/modules/README.mod_chmod](src/pre/modules/README.mod_chmod)
- [src/pre/modules/README.mod_idmp3](src/pre/modules/README.mod_idmp3)
- [src/pre/modules/README.mod_nfohandler](src/pre/modules/README.mod_nfohandler)
- [src/pre/modules/README.mod_sitenfoadd](src/pre/modules/README.mod_sitenfoadd)
- [src/pre/modules/README.mod_symlink](src/pre/modules/README.mod_symlink)

## TODO

Moved to [TODO.md](TODO.md)

