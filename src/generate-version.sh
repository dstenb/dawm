#! /bin/sh

echo "#ifndef _VERSION_H_" >  .version.h
echo "#define _VERSION_H_" >> .version.h
echo -n "#define VERSION \"" >> .version.h
git show -s --pretty=format:"commit %h (%ai)\"%n" >> .version.h
echo "#endif" >> .version.h

if [ ! -e version.h ] || [ "`diff -q version.h .version.h`" ]; then
	cp .version.h version.h
fi

rm .version.h
