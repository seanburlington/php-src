<?php

/* $Id: skipif.inc,v 1.2 2003/04/30 10:42:24 sniper Exp $ */
/* used in tests */

if (!extension_loaded("interbase")) print "skip"; 
if (!is_executable("isql") && !is_executable("/opt/interbase/bin/isql")) print "skip";

?>
