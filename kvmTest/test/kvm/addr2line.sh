#sampleAddr=$1
#watchAddr=$2

addr2line -f -e ./vmlinux $*

#$sampleAddr $watchAddr
