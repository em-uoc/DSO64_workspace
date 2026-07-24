savedcmd_kprobe.mod := printf '%s\n'   kprobe.o | awk '!x[$$0]++ { print("./"$$0) }' > kprobe.mod
