--TEST--
Bug #24142 (round() problems)
--FILE--
<?php // $Id: bug24142.phpt,v 1.3 2003/06/26 03:21:45 sterling Exp $ vim600:syn=php
echo round(5.045, 2). "\n";
echo round(5.055, 2). "\n";
?>
--EXPECT--
5.04
5.05
