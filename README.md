# foo-tools updates

- updates/fixes from https://github.com/glftpd/foo-tools
- Makefiles updated
- foo-pre has mp3 genre in PRE line (no module needed)

## Documentation:

- [README.1st](README.1st) Original README file
- [CHANGES](src/CHANGES) CHANGELOG
- [README.1st](src/README.1st) Building instructions
- [doc](doc) Documentation on installing/running the programs
- [README.mp3genre](src/pre/README.mp3genre) mp3genre specific README (install/configure)

## TODO:

### foo-pre
- [ ] switch from mp3genre to master branch
- [X] add option in pre.cfg disable genre completely(?)
- [ ] replace pre/mp3genre by pzs-ng code or other id3 lib instead
- [X] change bytes to mb/gb: "Totals of this pre for announce: 3F 28734928B"
- [ ] FLAC support?

### spy
- [ ] look into libhttpd.h since it's LGPL now (http://www.hughes.com.au/products/libhttpd)

