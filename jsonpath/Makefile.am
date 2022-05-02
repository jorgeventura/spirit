ACLOCAL_AMFLAGS=-stdc=++23 -I m4 -ftemplate-depth=5000

bin_PROGRAMS=jsp
noinst_LTLIBRARIES=libjsonpath.la

libjsonpath_la_SOURCES=jsonpath.cc

jsp_DEPENDENCIES=libjsonpath.la
jsp_LDADD=-L. -ljsonpath
jsp_SOURCES=jsp.cc jsonpath.hpp

