--TEST--
htmlentities() test 3 (setlocale / de_DE.ISO-8859-1)
--SKIPIF--
<?php
$result = (bool)setlocale(LC_CTYPE, "de_DE.ISO-8859-1", "de_DE.ISO8859-1");
if (!$result || preg_match('/ISO/i', setlocale(LC_CTYPE, 0)) == 0) {
	die("skip setlocale() failed\n");
}
?>
--INI--
output_handler=
default_charset=
mbstring.internal_encoding=none
--FILE--
<?php
	setlocale( LC_CTYPE, "de_DE.ISO-8859-1", "de_DE.ISO8859-1");
	var_dump(htmlentities("\xe4\xf6\xfc", ENT_QUOTES, ''));
?>
--EXPECTF--
Deprecated: setlocale(): deprecated in Unicode mode, please use ICU locale functions in %s on line %d
unicode(18) "&auml;&ouml;&uuml;"
