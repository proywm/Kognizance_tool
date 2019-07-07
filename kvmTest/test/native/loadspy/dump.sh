start=$1
end=$2
objdump -z -d --start-address=$start  --stop-address=$end ./test1
