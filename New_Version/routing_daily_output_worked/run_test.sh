#!/bin/bash

ctlfile="temporary_ctl.txt"


basin=1
nbasin=1
while [ $basin -le $nbasin ]; do

	inputfile="../setup_input_files/input_basin_${basin}.bin"
	vicfile="../setup_vic_input_files/vic_input_basin_${basin}.bin"
	year=1997
	while [ $year -le 2004 ]; do 

		cat $ctlfile |  sed 's/YEAR/'${year}'/g' | sed 's/BASIN/'${basin}'/g' > ctlfile.txt

		./route ctlfile.txt $inputfile $vicfile

		let year=year+1
	done
	
	let basin=basin+1
done
