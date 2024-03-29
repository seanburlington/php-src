--TEST--
Test rename() function : variation - various invalid paths
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--SKIPIF--
<?php
if(substr(PHP_OS, 0, 3) == "WIN")
  die("skip Not for Windows");
?>
--FILE--
<?php
/* Prototype  : bool rename(string old_name, string new_name[, resource context])
 * Description: Rename a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */


echo "*** Testing rename() with obscure files ***\n";
$file_path = dirname(__FILE__)."/renameVar13";
$aFile = $file_path.'/afile.tmp';

mkdir($file_path);

/* An array of files */ 
$names_arr = array(
  /* Invalid args */ 
  -1,
  TRUE,
  FALSE,
  NULL,
  "",
  " ",
  "\0",
  array(),

  /* prefix with path separator of a non existing directory*/
  "/no/such/file/dir", 
  "php/php"

);

for( $i=0; $i<count($names_arr); $i++ ) {
  $name = $names_arr[$i];
  echo "-- testing '$name' --\n";  
  touch($aFile);
  var_dump(rename($aFile, $name));
  if (file_exists($name)) {
     unlink($name);
  }
  if (file_exists($aFile)) {
     unlink($aFile);
  }
  var_dump(rename($name, $aFile));
  if (file_exists($aFile)) {
     unlink($aFile);
  }
}

rmdir($file_path);
echo "\n*** Done ***\n";
?>
--EXPECTF--
*** Testing rename() with obscure files ***
-- testing '-1' --
bool(true)

Warning: rename(-1,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing '1' --
bool(true)

Warning: rename(1,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing '' --

Warning: rename(%s/renameVar13/afile.tmp,): No such file or directory in %s on line %d
bool(false)

Warning: rename(,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing '' --

Warning: rename(%s/renameVar13/afile.tmp,): No such file or directory in %s on line %d
bool(false)

Warning: rename(,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing '' --

Warning: rename(%s/renameVar13/afile.tmp,): No such file or directory in %s on line %d
bool(false)

Warning: rename(,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing ' ' --
bool(true)

Warning: rename( ,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing ' ' --

Warning: rename(%s/renameVar13/afile.tmp,): No such file or directory in %s on line %d
bool(false)

Warning: rename(,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)

Notice: Array to string conversion in %s on line %d
-- testing 'Array' --

Notice: Array to string conversion in %s on line %d
bool(true)

Warning: file_exists() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d

Notice: Array to string conversion in %s on line %d
bool(true)
-- testing '/no/such/file/dir' --

Warning: rename(%s/renameVar13/afile.tmp,/no/such/file/dir): No such file or directory in %s on line %d
bool(false)

Warning: rename(/no/such/file/dir,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)
-- testing 'php/php' --

Warning: rename(%s/renameVar13/afile.tmp,php/php): No such file or directory in %s on line %d
bool(false)

Warning: rename(php/php,%s/renameVar13/afile.tmp): No such file or directory in %s on line %d
bool(false)

*** Done ***

