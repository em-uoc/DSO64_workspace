#!/bin/bash

insmod sysdemo.ko
cat /sys/module/sysdemo/parameters/target_pid 
echo 1 > /sys/module/sysdemo/parameters/target_pid 
cat /sys/kernel/sysdemo/ppid 
cat /sys/kernel/sysdemo/nvcsw 
cat /sys/kernel/sysdemo/nivcsw 
cat /sys/kernel/sysdemo/pgd
echo 123456 > /sys/module/sysdemo/parameters/target_pid 
cat /sys/kernel/sysdemo/pgd
echo -123456 > /sys/module/sysdemo/parameters/target_pid 
rmmod sysdemo

insmod sysdemo.ko target_pid=5
cat /sys/module/sysdemo/parameters/target_pid 
rmmod sysdemo

