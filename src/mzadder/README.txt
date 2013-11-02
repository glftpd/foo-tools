
Quick installation guide for Muggi Zip Adder v2.0
~

1) "make build" in .. to build dependencies.

2) "make" to build mzadder binary

3) Copy mzadder binary to your doors/ directory.

4) Edit sample.cfg to suit your needs, and copy it somewhere.

5) Add mzadder to examine.dat for the conferences you want it used in.
   example line in examine.dat:

---
/home/bbs/doors/mzadder %N /home/bbs/configs/mzadder.cfg
---

6) Go upload a file and check everything works out ok.

7) The mzadder can be tested without DD by setting the node to -t, and supplying filename on
commandline, eg.

./mzadder -t sample.cfg test.zip


Macros
~

In the ads there are the following macros available:

FN - filename (string)
FS - file size (MB) (float)
UN - uploader name (string)
UL - uploader location (string)
NODE - node number (string)
DATE - current date (string)
TIME - current time (string)
STIM - current time (without seconds) (string)

Macros are used as follows:

%[format]macroname%

Format is standard printf format for the datatype (see list above, string or float: %s or %f)

Examples (see also bbs1.nfo in the dist for an example of use):

%[%-11.11s]FN% (filename left-aligned in a 11 width field)
%[%.1f]FS%     (filesize shown with 1 decimal)
%[%s]DATE%     (the upload date)
