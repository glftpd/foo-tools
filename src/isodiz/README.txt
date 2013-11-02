
isodiz (c) tanesha 2009
-----------------------

A tool to create "file_id.diz" from FTP uploaded RAR/SFV files.

It is supposed to be integrated into DayDream, but since I do not
have any DayDream installation, the usability of this is undetermined.

Currenly, isodiz is part of the footools package, and must as such be
compiled together with it.


Usage
-----

It can work in two modes. First is dir-mode, where information is read
from the directory where the file is found, and second, where a glftpd
"dirlog" is consulted.


dir-mode
--------

A .sfv file is consulted in the directory to find number of files in
the release.

Examples:
/path/to/isodiz -d -f /path/to/Some.Release.Name-GRP/file.sfv -o file_id.diz
/path/to/isodiz -d -f /path/to/Some.Release.Name-GRP/file.r01 -o file_id.diz
/path/to/isodiz -d -f /path/to/Some.Release.Name-GRP/file.r02 -o -


glftpd dirlog mode
------------------

The glftpd dirlog is consulted to find number of files in the release.

Examples:
/path/to/isodiz -g /glftpd -s /site \
	-f /glftpd/site/rls/Some.Release.Name-GRP/file.sfv -o file_id.diz
/path/to/isodiz -g /glftpd -s /site \
	-f /glftpd/site/rls/Some.Release.Name-GRP/file.rar -o file_id.diz
/path/to/isodiz -g /glftpd -s /site \
	-f /glftpd/site/rls/Some.Release.Name-GRP/file.r00

/path/to/isodiz -f /glftpd/site/rls/Some.Release.Name-GRP/file.r01


Parameters
----------

 -f /path/to/upload/fil  Specifies the file to generate file_id for.
                         [Mandatory parameter]

 -g /path/to/glftpd      Specifies the path for the glftpd installation.
                         [default: /glftpd]

 -s /site-dir            Specifies the /site folder inside the glftpd
                         installation  [default: /site]

 -d                      Specifies to use "dir-mode" [default: No dir-mode]

 -o /path/to/filename    Specifies output file to use. "-" means stdout.
                         [default: "-"]

 -z /path/to/dizs        Specifies a folder where to search for group.diz
                         If a file "group.diz" (eg. heirloom.diz) is found
                         in this folder, then the contents will be appended
                         to the output.
                         [default: not used]

 -a                      Adds file_id.diz to the SFV as comment with 
                         @BEGIN/@END tags.
