dnl $Id: config.m4,v 1.5.2.1 2000/12/06 17:41:40 sas Exp $
dnl config.m4 for PHP4 CCVS Extension

AC_MSG_CHECKING(CCVS Support)
AC_ARG_WITH(ccvs,
[  --with-ccvs[=DIR]       Compile CCVS support into PHP4. Please specify your 
                          CCVS base install directory as DIR.],
[
  if test "$withval" != "no"; then
     CCVS_DIR="$withval"
	test -f $withval/include/cv_api.h && CCVS_INCLUDE_DIR="$withval/include"
    test -f $withval/lib/libccvs.a && CCVS_LIB_DIR="$withval/lib"

     	if test -n "$CCVS_DIR"; then
		AC_MSG_RESULT(yes)
		PHP_EXTENSION(ccvs)
		LIBS="$LIBS -L$CCVS_LIB_DIR"
		AC_ADD_LIBRARY_WITH_PATH(ccvs, $CCVS_LIB_DIR)
		AC_ADD_INCLUDE($CCVS_INCLUDE_DIR)
    	  else
    	    AC_MSG_RESULT(no)
    	  fi
	fi
],[
  AC_MSG_RESULT(no)
])
