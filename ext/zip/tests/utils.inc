<?php
/* $Id: utils.inc,v 1.2 2006/10/30 17:43:21 pajoye Exp $ */
function dump_entries_name($z) {
	for($i=0; $i<$z->numFiles; $i++) {
	    $sb = $z->statIndex($i);
	    echo $i . ' ' . $sb['name'] . "\n";
	}
}

/* recursively remove a directoryy */
function rmdir_rf($dir) {
	if ($handle = opendir($dir)) {
		while (false !== ($item = readdir($handle))) {
			if ($item != "." && $item != "..") {
				if (is_dir($dir . '/' . $item)) {
					rmdir_rf($dir . '/' . $item);
				} else {
					unlink($dir . '/' . $item);
				}
			}
		}
		closedir($handle);
		rmdir($dir);
	}
}
