--TEST--
Bug #24142 (round() problems)
--FILE--
<?php // $Id: bug24142.phpt,v 1.2 2003/06/16 19:59:27 sniper Exp $ vim600:syn=php
echo round(5.045, 2). "\n";
echo round(5.055, 2). "\n";
?>
--EXPECT--
5.05
5.06
