--TEST--
ReflectionClass::getExtension() method - basic test for getExtension() method
--SKIPIF--
<?php extension_loaded('dom') or die('skip - dom extension not loaded'); ?>
--CREDITS--
Rein Velt <rein@velt.org>
#testFest Roosendaal 2008-05-10
--FILE--
<?php
 	$rc=new reflectionClass('domDocument');
 	var_dump($rc->getExtension()) ;
?>
--EXPECTF--
object(ReflectionExtension)#%d (1) {
  [u"name"]=>
  unicode(3) "dom"
}
