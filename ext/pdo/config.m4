dnl $Id: config.m4,v 1.4 2004/05/19 13:28:04 edink Exp $
dnl config.m4 for extension pdo

PHP_ARG_ENABLE(pdo, whether to enable PDO support,
[  --enable-pdo            Enable PHP Data Objects support])

if test "$PHP_PDO" != "no"; then
  PHP_NEW_EXTENSION(pdo, pdo.c pdo_dbh.c pdo_stmt.c pdo_sql_parser.c, $ext_shared)
  PHP_ADD_MAKEFILE_FRAGMENT
fi
