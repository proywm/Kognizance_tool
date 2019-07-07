
sudo gdb /home/probir/qemu/qemu_pinning/build/x86_64-softmmu/qemu-system-x86_64

set exec-wrapper env 'LD_PRELOAD=/home/probir/Downloads/kpv_temp/temp/kvmTest/test/kvm/../../build_kvm/libperform.so:/home/probir/Downloads/kpv_temp/temp/kpv_temp/dependencies/libmonitor/installDir/lib/libmonitor.so'

r -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=2,threads=2,sockets=1 -vcpu vcpunum=0,affinity=0 -vcpu vcpunum=1,affinity=1 -vcpu vcpunum=2,affinity=2 -vcpu vcpunum=3,affinity=3 -accel accel=kvm -redir tcp:2222::22 -device hyperf


