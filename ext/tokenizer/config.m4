dnl $Id: config.m4,v 1.6 2007/07/31 23:23:37 johannes Exp $
dnl config.m4 for extension tokenizer

dnl Otherwise use enable:

PHP_ARG_ENABLE(tokenizer, whether to enable tokenizer support,
[  --disable-tokenizer     Disable tokenizer support], yes)

if test "$PHP_TOKENIZER" != "no"; then
  PHP_NEW_EXTENSION(tokenizer, tokenizer.c tokenizer_data.c, $ext_shared)
  PHP_ADD_MAKEFILE_FRAGMENT
fi
