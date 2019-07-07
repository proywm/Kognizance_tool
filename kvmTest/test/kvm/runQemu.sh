#sudo sh run.sh /home/proy/witch/perform/qemu/qemu/build_perform/x86_64-softmmu/qemu-system-x86_64  -enable-kvm -drive format=raw,file=/home/proy/witch/perform/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=2 -device hyperf,dma_mask=32 -redir tcp:2222::22

#sudo sh run.sh /home/proy/witch/perform/qemu/qemu/build_perform/x86_64-softmmu/qemu-system-x86_64  -enable-kvm -drive format=raw,file=/home/proy/witch/perform/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=2 -device hyperf -redir tcp:2222::22

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf -nographic

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64  -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf -nographic

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf -nographic

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf 

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=2,threads=2,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf

sudo sh run.sh /home/probir/qemu/qemu_pinning/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=2,threads=2,sockets=1 -vcpu vcpunum=0,affinity=0 -vcpu vcpunum=1,affinity=1 -vcpu vcpunum=2,affinity=2 -vcpu vcpunum=3,affinity=3 -accel accel=kvm -redir tcp:2222::22 -device hyperf 

#sudo /home/probir/qemu/qemu/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=2,threads=2,sockets=1 -accel accel=kvm -redir tcp:2222::22

#sudo sh run.sh /home/probir/qemu/qemu_pinning/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=2,threads=2,sockets=1 -vcpu vcpunum=0,affinity=0 -vcpu vcpunum=1,affinity=1 -vcpu vcpunum=2,affinity=2 -vcpu vcpunum=3,affinity=3 -accel accel=kvm -redir tcp:2222::22 -device hyperf -nographic

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -s -S -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS2.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 -device hyperf -nographic

#-nographic -serial mon:stdio

#sudo sh run.sh /home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22 


#/home/probir/qemu/qemu_kvmTest/build/x86_64-softmmu/qemu-system-x86_64 -enable-kvm -drive format=raw,file=/home/probir/kvmtool/ubuntu_disk/ubuntuS.img --device virtio-balloon -m 4096M -smp cpus=4,cores=4,threads=1,sockets=1 -accel accel=kvm -redir tcp:2222::22
