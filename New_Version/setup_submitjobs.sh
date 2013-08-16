#!/bin/bash

basin=$1
inputfile="./setup_input_files/input_basin_${basin}.bin"
vicfile="./setup_vic_input_files/vic_input_basin_${basin}.bin"

cat > ./tmp/route.sh.${basin} << EOF
#!/bin/sh
./routing_daily_output_worked/route ./tmp/ctlfile_${basin}.txt $inputfile $vicfile

EOF

exit
