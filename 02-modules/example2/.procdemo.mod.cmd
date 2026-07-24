savedcmd_procdemo.mod := printf '%s\n'   procdemo.o | awk '!x[$$0]++ { print("./"$$0) }' > procdemo.mod
