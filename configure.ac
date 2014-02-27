#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ns], [0.1.0], [me@jacobdegeling.com])
AC_CONFIG_SRCDIR([src/file.c])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AH_TOP([#define _GNU_SOURCE  1])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC

# Checks for libraries.
AC_CHECK_LIB([argp], [argp_parse])
AC_CHECK_LIB([exif], [exif_data_new_from_file])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strrchr])

AC_MSG_CHECKING([what the host OS is])
case $(uname -s) in
    Darwin)
        AC_MSG_RESULT([Darwin])
        AM_CONDITIONAL([DARWIN], true)
        AM_CONDITIONAL([LINUX], false)
        AM_CONDITIONAL([UNKNOWN], false)
        ;;
    Linux)
        AC_MSG_RESULT([Linux])
        AM_CONDITIONAL([LINUX], true)
        AM_CONDITIONAL([DARWIN], false)
        AM_CONDITIONAL([UNKNOWN], false)
        ;;
    *)
        AC_MSG_RESULT([Unknown OS])
        AM_CONDITIONAL([UNKNOWN], true)
        AM_CONDITIONAL([LINUX], false)
        AM_CONDITIONAL([DARWIN], false)
        ;;
esac

AC_CONFIG_FILES([makefile src/makefile])
AC_OUTPUT
