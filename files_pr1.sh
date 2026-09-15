#!/bin/bash

rm -f workspace.zip
ln -s . workspace

zip workspace.zip \
	workspace/01-newsyscalls/example1/kernel/newsyscall.c \
	workspace/01-newsyscalls/example1/user/Makefile \
	workspace/01-newsyscalls/example1/user/test1.c \
	workspace/01-newsyscalls/exercise_nch/kernel/Makefile \
	workspace/01-newsyscalls/exercise_nch/kernel/README \
	workspace/01-newsyscalls/exercise_nch/user/Makefile \
	workspace/01-newsyscalls/exercise_nch/user/test_nch.c \
	workspace/01-newsyscalls/exercise_nch/user/get_nch.c \
	workspace/01-newsyscalls/exercise_nch/user/get_nch.h \
	workspace/02-modules/example1/Makefile \
	workspace/02-modules/example1/hw.c \
	workspace/02-modules/example2/Makefile \
	workspace/02-modules/example2/procdemo.c \
	workspace/02-modules/example3/Makefile \
	workspace/02-modules/example3/sysdemo.c \
	workspace/02-modules/example4/Makefile \
	workspace/02-modules/example4/kprobe.c \
	workspace/02-modules/exercise_kprobe_write/Makefile \
	workspace/02-modules/exercise_kprobe_write/test_wr.c \
	workspace/02-modules/exercise_sysfs/Makefile \
	workspace/02-modules/exercise_sysfs/test_nch.c \
	workspace/02-modules/exercise_sysfs/get_nch.h 
