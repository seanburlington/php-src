--TEST--
Gettext basic test
--SKIPIF--
<?php if (!extension_loaded("gettext")) print "skip"; ?>
--FILE--
<?php // $Id: gettext_basic.phpt,v 1.1 2003/09/23 10:00:23 sniper Exp $

chdir(dirname(__FILE__));
setlocale(LC_ALL, 'fi_FI');
bindtextdomain ("messages", "./locale");
textdomain ("messages");
echo gettext("Basic test"), "\n";
echo _("Basic test"), "\n";

?>
--EXPECT--
Perustesti
Perustesti
