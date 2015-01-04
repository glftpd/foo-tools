#!/bin/sh


rm foo-pre
make
if [ -f "foo-pre" ]; then
  echo "chowning.."
  chown root foo-pre
  echo "chmoding.."
  chmod 4711 foo-pre
  echo "copying.."
  cp foo-pre /glftpd2/bin/foo-pre
  echo -e "\n\ndone."
else
  echo -e "\n\nerror compliing. installation aborted."
  exit 1
fi
