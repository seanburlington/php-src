dnl
dnl $Id: config.m4,v 1.3 2002/01/24 17:55:13 edink Exp $
dnl

AC_MSG_CHECKING(for CLI build)

AC_ARG_ENABLE(cli,
[  --disable-cli           Disable building CLI version of PHP.],
[
  if test "$enable_cli" != "no"; then
    PHP_SAPI_CLI=yes
  else
    PHP_SAPI_CLI=no
  fi
],
[PHP_SAPI_CLI=yes]
)

if test "$PHP_SAPI_CLI" != "no"; then
  INSTALL_IT="$INSTALL_IT; \$(INSTALL) -m 0755 sapi/cli/php \$(INSTALL_ROOT)\$(bindir)/php"
else
  PHP_DISABLE_CLI
fi

AC_MSG_RESULT($PHP_SAPI_CLI)
