dnl $Id: config.m4,v 1.1 2004/05/17 15:41:51 wez Exp $
dnl config.m4 for extension pdo

PHP_ARG_ENABLE(pdo, whether to enable PDO support,
[  --enable-pdo           Enable PHP Data Objects support])

if test "$PHP_PDO" != "no"; then
  PHP_NEW_EXTENSION(pdo, pdo.c pdo_dbh.c pdo_stmt.c, $ext_shared)
fi
