<?php
/* This is a list of packages and versions
 * that will be used to create the PEAR folder
 * in the windows snapshot.
 * See win32/build/mkdist.php for more details
 * $Id: go-pear-list.php,v 1.7 2004/04/09 01:12:41 cellog Exp $
 */
$packages  = array(
// required packages for the installer
"PEAR"                  =>    "1.3.1",
"XML_RPC"               =>    "1.1.0",
"Console_Getopt"        =>    "1.2",
"Archive_Tar"           =>    "1.1",

// required packages for the web frontend
"PEAR_Frontend_Web"     =>    "0.3"
"Pager"                 =>    "1.0.8",
"HTML_Template_IT"      =>    "1.1",
"Net_UserAgent_Detect"  =>    "1.0",
);

?>
