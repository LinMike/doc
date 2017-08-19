# capture_manager

## How to debug segment fault
1. build with the app with debug flag (see Makefile)
2. run "ulimit -c 1024" in arm
3. run app, if segment fault happens, a file named "core" will be generated
4. in vm: "sudo chown yuanf:yuanf ~/nfs/core" to make sure there is read permission for gdb
5. in vm: "arm-linux-gdb ~/nfs/capture_manager ~/nfs/core"
