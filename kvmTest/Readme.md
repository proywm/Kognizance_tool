
Build:

Python Agent: cmake -DPYAGENT=TRUE ..
Native Agent: cmake ..

WitchTool: cmake -DWITCHTOOL=TRUE ..

python + WitchTool: cmake -DWITCHTOOL=TRUE -DPYAGENT=TRUE ..

kvm: cmake -DKVMAGENT=TRUE ..

KVM + Witch: cmake -DKVMAGENT=TRUE -DWITCHTOOL=TRUE ..

KVM + PEBSBP : cmake -DKVMAGENT=TRUE -DSAMPLEPEBSBP=TRUE ..

native + PEBSBP: cmake -DSAMPLEPEBSBP=TRUE ..
	


export LD_PRELOAD=/home/probir/Downloads/test_python_mod/perform/build/libperform.so:/home/probir/Downloads/perform/dependencies/libmonitor/installDir/lib/libmonitor.so:$LD_PRELOAD
