--TEST--
PDO_OCI: connect
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("pdo_oci")) print "skip"; 
?>
--FILE--
<?php /* $Id: connect.phpt,v 1.1 2004/06/14 20:10:27 tony2001 Exp $ */

require "settings.inc";

$db = new PDO("oci:dbname=$dbase",$user,$password) or die("connect error");
echo "done\n";
	
?>
--EXPECT--
done
