#!/bin/bash

fdir='/home/frost/ckf/Documents/Data/HydroSHEDS/30sec/NA/na_dir_30s_bil/na_dir_30s.bil'
fflow='/home/frost/ckf/Documents/Data/HydroSHEDS/30sec/NA/na_acc_30s_bil/na_acc_30s.bil'
felev='/home/frost/ckf/Documents/Data/HydroSHEDS/30sec/NA/na_dem_30s_bil/na_dem_30s.bil'
flowthresh=40

# Get info
fileinfo="latlon_list.txt"
nlines=`wc -l $fileinfo | awk '{print $1}'`
n=1
while [ $n -le $nlines ]; do

	echo $n

	basinnum=`cat $fileinfo | awk '{if(NR=='${n}') print $1}'`
	mlat=`cat $fileinfo | awk '{if(NR=='${n}') print $2}'`
	mlon=`cat $fileinfo | awk '{if(NR=='${n}') print $3}'`	
	fout="./input_basin_${basinnum}.bin"
	fout2="./basin_${basinnum}_latlon.txt"

	./setup_input_files $fdir $fflow $felev $fout $fout2 $flowthresh $mlat $mlon
	
	let n=n+1
done
