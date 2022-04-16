#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([spirit], [0.1.0], [jv120257@gmail.com])
AC_CONFIG_SRCDIR([inline/inline_parser.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE

# Checks for programs.
LT_INIT
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([boost_json],[main],[],[AC_MSG_FAILURE("Install libboost-json package")],[])ac_cv_lib_boost_json=ac_cv_lib_boost_json_main

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
	Makefile
	inline/Makefile
	roman/Makefile
	variant/Makefile
	employee/Makefile
	emplnofusion/Makefile
	calc1/Makefile
	calc2/Makefile
	calc3/Makefile
	calc4/Makefile
	calc5/Makefile
	jsonpath/Makefile
])
AC_OUTPUT
