#!/bin/bash

rm -f workspace.zip
zip workspace.zip \
	01-newsyscalls/example1/kernel/newsyscall.c \
	01-newsyscalls/example1/user/Makefile \
	01-newsyscalls/example1/user/test1.c \
	01-newsyscalls/exercise_nch/kernel/Makefile \
	01-newsyscalls/exercise_nch/kernel/README \
	01-newsyscalls/exercise_nch/user/Makefile \
	01-newsyscalls/exercise_nch/user/test_nch.c \
	01-newsyscalls/exercise_nch/user/get_nch.c \
	01-newsyscalls/exercise_nch/user/get_nch.h \
	02-modules/example1/Makefile \
	02-modules/example1/hw.c \
	02-modules/example2/Makefile \
	02-modules/example2/procdemo.c \
	02-modules/example3/Makefile \
	02-modules/example3/sysdemo.c \
	02-modules/example4/Makefile \
	02-modules/example4/kprobe.c \
	02-modules/exercise_kprobe_write/Makefile \
	02-modules/exercise_kprobe_write/test_wr.c \
	02-modules/exercise_sysfs/Makefile \
	02-modules/exercise_sysfs/test_nch.c \
	02-modules/exercise_sysfs/get_nch.h 
