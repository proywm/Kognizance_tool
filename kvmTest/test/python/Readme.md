#export LD_LIBRARY_PATH=../../build/:$LD_LIBRARY_PATH


export LD_PRELOAD=/home/proy/witch/perform/kpv_temp/build_py/libperform.so:/home/proy/witch/perform/kpv_temp/dependencies/libmonitor/installDir/lib/libmonitor.so:$LD_PRELOAD

cp ../../build_py/*.so .

