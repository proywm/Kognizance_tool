start=$1
end=$2
objdump -z -d -S -D --start-address=0x$start  --stop-address=0x$end ./vmlinux
