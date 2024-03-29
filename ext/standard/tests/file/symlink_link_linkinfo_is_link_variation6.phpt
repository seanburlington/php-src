--TEST--
Test symlink(), linkinfo(), link() and is_link() functions : usage variations - link & dir perms.
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
    die('skip no symlinks on Windows');
}
if (substr(PHP_OS, 0, 3) == 'MAC') {
    die('skip not valid for MacOS');
}
// Skip if being run by root (files are always readable, writeable and executable)
$filename = dirname(__FILE__)."/symlink_link_linkinfo_is_link6_check_root.tmp";
$fp = fopen($filename, 'w');
fclose($fp);
if(fileowner($filename) == 0) {
        unlink ($filename);
        die('skip cannot be run as root');
}

unlink($filename);

?>
--FILE--
<?php
/* Prototype: bool symlink ( string $target, string $link );
   Description: creates a symbolic link to the existing target with the specified name link

   Prototype: bool is_link ( string $filename );
   Description: Tells whether the given file is a symbolic link.

   Prototype: bool link ( string $target, string $link );
   Description: Create a hard link

   Prototype: int linkinfo ( string $path );
   Description: Gets information about a link
*/

/* Variation 6 : Change permission of directory and try creating links inside that directory */
$file_path = dirname(__FILE__);

echo "*** Creating links in a directory without permission to allow the operation ***\n";
// temp file used
$dirname = "$file_path/symlink_link_linkinfo_is_link_variation6";
mkdir($dirname);
$filename = "$dirname/symlink_link_linkinfo_is_link_variation6.tmp";

// remove all permissions from dir
var_dump( chmod($dirname, 0000) );

echo "\n-- Working with soft links --\n";
$linkname = "$dirname/non_existent_link_variation5.tmp";

// expected: false
var_dump( symlink($filename, $linkname) ); // this link won't get created 
var_dump( linkinfo($linkname) );
var_dump( is_link($linkname) );
// clear the cache
clearstatcache();

echo "\n-- Working with hard links --\n";
// expected: false
var_dump( link($filename, $linkname) );
var_dump( linkinfo($linkname) );
var_dump( is_link($linkname) );
// clear the cache
clearstatcache();

chmod($dirname, 0777);  // to enable dir deletion
echo "Done\n";
?>
--CLEAN--
<?php
$file_path = dirname(__FILE__);
$dirname = "$file_path/symlink_link_linkinfo_is_link_variation6";
$filename = "$dirname/symlink_link_linkinfo_is_link_variation6.tmp";
unlink($filename);
rmdir($dirname);
?>
--EXPECTF--
*** Creating links in a directory without permission to allow the operation ***
bool(true)

-- Working with soft links --

Warning: symlink(): Permission denied in %s on line %d
bool(false)

Warning: linkinfo(): Permission denied in %s on line %d
int(-1)
bool(false)

-- Working with hard links --

Warning: link(): Permission denied in %s on line %d
bool(false)

Warning: linkinfo(): Permission denied in %s on line %d
int(-1)
bool(false)
Done
