#!/bin/bash

n=1
nbasin=15
while [ $n -le $nbasin ]; do 
	river_network="../setup_input_files/input_basin_${n}.bin"
	basin_latlon="../setup_input_files/basin_${n}_latlon.txt"
        output="vic_input_basin_${n}.bin"
	resn=0.25
        nlines=`wc -l $basin_latlon | awk '{print $1}'`
	./setup_vic_input_files $river_network $basin_latlon $output $resn $nlines  

	let n=n+1
done
