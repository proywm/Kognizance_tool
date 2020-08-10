
Build:

mkdir build_native

Python Agent: cmake -DPYAGENT=TRUE ..
Native Agent: cmake ..

WitchTool: cmake -DWITCHTOOL=TRUE ..

python + WitchTool: cmake -DWITCHTOOL=TRUE -DPYAGENT=TRUE ..

kvm: cmake -DKVMAGENT=TRUE ..

KVM + Witch: cmake -DKVMAGENT=TRUE -DWITCHTOOL=TRUE ..

KVM + PEBSBP : cmake -DKVMAGENT=TRUE -DSAMPLEPEBSBP=TRUE ..

native + PEBSBP: cmake -DSAMPLEPEBSBP=TRUE ..
	
native + PEBS: cmake -DSAMPLEPEBS=TRUE ..

export LD_PRELOAD=/home/Kognizance_tool/kvmTest/build_native/libperform.so:/home/Kognizance_tool/kvmTest/dependencies/libmonitor/installDir/lib/libmonitor.so:$LD_PRELOAD
