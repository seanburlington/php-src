<?php
/* This is a list of packages and versions
 * that will be used to create the PEAR folder
 * in the windows snapshot.
 * See win32/build/mkdist.php for more details
 * $Id: go-pear-list.php,v 1.9.2.2 2004/10/28 19:31:07 cellog Exp $
 */
$packages  = array(
// required packages for the installer
"PEAR"                  =>    "1.3.3",
"XML_RPC"               =>    "1.1.0",
"Console_Getopt"        =>    "1.2",
"Archive_Tar"           =>    "1.2",

// required packages for the web frontend
"PEAR_Frontend_Web"     =>    "0.4",
"HTML_Template_IT"      =>    "1.1",
"Net_UserAgent_Detect"  =>    "1.0",
);

?>
