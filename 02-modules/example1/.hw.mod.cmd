savedcmd_hw.mod := printf '%s\n'   hw.o | awk '!x[$$0]++ { print("./"$$0) }' > hw.mod
