--TEST--
shmop extension test
--SKIPIF--
<?php
	if (!extension_loaded("shmop")) {
		die("skip shmop() extension not available");
	}
?>
--FILE--
<?php
	$hex_shm_id = 0xff3;
	$write_d1 = b"test #1 of the shmop() extension";
	$write_d2 = b"test #2 append data to shared memory segment";

	echo "shm open for create: ";
	$shm_id = shmop_open($hex_shm_id, "n", 0644, 1024);   
	if (!$shm_id) {
		die("failed\n");
	} else {
		echo "ok\n";
	}
	
	echo "shm size is: " . ($shm_size = shmop_size($shm_id)) . "\n";
	
	echo "shm write test #1: ";
	$written = shmop_write($shm_id, $write_d1, 0);
	if ($written != strlen($write_d1)) {
		echo "failed\n";
	} else {
		echo "ok\n";
	}
	
	echo "data in memory is: " . shmop_read($shm_id, 0, $written) . "\n";

	shmop_close($shm_id);
	
	echo "shm open for read only: ";
	$shm_id = shmop_open($hex_shm_id, "a", 0644, 1024);   
	if (!$shm_id) {
		echo "failed\n";
	} else {
		echo "ok\n";
	}
	
	echo "data in memory is: " . shmop_read($shm_id, 0, $written) . "\n";
	
	/* try to append data to the shared memory segment, this should fail */
	@shmop_write($shm_id, $write_d1, $written);
	echo $php_errormsg . "\n";

	shmop_close($shm_id);
	
	echo "shm open for read only: ";
	$shm_id = shmop_open($hex_shm_id, "w", 0644, 1024);   
	if (!$shm_id) {
		echo "failed\n";
	} else {
		echo "ok\n";
	}
	
	echo "shm write test #1: ";
	$written = shmop_write($shm_id, $write_d2, $written);
	if ($written != strlen($write_d2)) {
		die("failed\n");
	} else {
		echo "ok\n";
	}
	
	echo "data in memory is: " . shmop_read($shm_id, 0, strlen($write_d1 . $write_d2)) . "\n";

	echo "deletion of shm segment: ";
	if (!shmop_delete($shm_id)) {
		echo "failed\n";
	} else {
		echo "ok\n";
	}
	
	shmop_close($shm_id);
?>
--EXPECT--
shm open for create: ok
shm size is: 1024
shm write test #1: ok
data in memory is: test #1 of the shmop() extension
shm open for read only: ok
data in memory is: test #1 of the shmop() extension
trying to write to a read only segment
shm open for read only: ok
shm write test #1: ok
data in memory is: test #1 of the shmop() extensiontest #2 append data to shared memory segment
deletion of shm segment: ok
