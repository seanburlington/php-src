--TEST--
htmlentities() test 12 (default_charset / ISO-8859-1)
--INI--
output_handler=
mbstring.internal_encoding=pass
default_charset=ISO-8859-1
--FILE--
<?php
	print ini_get('default_charset')."\n";
	var_dump(htmlentities("\xe4\xf6\xfc", ENT_QUOTES, ''));
?>
--EXPECT--
ISO-8859-1
unicode(18) "&auml;&ouml;&uuml;"
