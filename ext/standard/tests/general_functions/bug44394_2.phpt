--TEST--
Bug #44394 (Last two bytes missing from output) with session.use_trans_id
--SKIPIF--
<?php if (!extension_loaded("session")) print "skip"; ?>
--INI--
session.name=PHPSESSID
--FILE--
<?php

ini_set('session.use_trans_sid', 1);
session_save_path(__DIR__);
session_start();

ob_start();

$string = "<a href='a?q=1'>asd</a>";

output_add_rewrite_var('a', 'b');

echo $string;

ob_flush();

ob_end_clean();

?>
--CLEAN--
<?php
unlink(__DIR__ . '/sess_'.session_id());
foreach (glob(__DIR__ . '/sess_*') as $filename) {
  unlink($filename);
}
?>
--EXPECTF--
<a href='a?q=1&PHPSESSID=%s&a=b'>asd</a>
