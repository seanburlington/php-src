--TEST--
preg_match() single line match with multi-line input
--FILE--
<?php
/* Prototype  : int preg_match  ( string $pattern  , string $subject  [, array &$matches  [, int $flags  [, int $offset  ]]] )
 * Description: Perform a regular expression match
 * Source code: ext/pcre/php_pcre.c
 */
 
$string = "My\nName\nIs\nStrange";
preg_match("/M(.*)/", $string, $matches);

var_dump($matches);
?>
===Done===
--EXPECTF--
array(2) {
  [0]=>
  unicode(2) "My"
  [1]=>
  unicode(1) "y"
}
===Done===