--TEST--
Test ereg() function : usage variations - pass non-variable as arg 3, which is pass-by-ref.
--FILE--
<?php
/* Prototype  : proto int ereg(string pattern, string string [, array registers])
 * Description: Regular expression match 
 * Source code: ext/standard/reg.c
 * Alias to functions: 
 */

var_dump(ereg(b'l{2}', b'hello', str_repeat('x',1)));
echo "Done";
?>
--EXPECTF--

Strict Standards: Only variables should be passed by reference in %s on line 8
int(2)
Done