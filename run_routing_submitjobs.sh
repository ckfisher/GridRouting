#!/bin/bash

ctlfile="route.ctl"


basin=12
nbasin=12
while [ $basin -le $nbasin ]; do

	inputfile="~/setup_input_files/input_basin_${basin}.bin"
	vicfile="~/setup_vic_input_files/vic_input_basin_${basin}.bin"

       cat $ctlfile |  sed 's/BASIN/'${basin}'/g' > ./tmp/ctlfile_${basin}.txt

       bash ./setup_submitjobs.sh ${basin}

       bash ./tmp/route.sh.${basin}	
       let basin=basin+1
done
