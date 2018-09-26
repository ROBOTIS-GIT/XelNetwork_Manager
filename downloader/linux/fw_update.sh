#! /bin/bash

shell_cmd="./xel_loader"

echo "XEL Firmware Update Start.."

if (($#==2))
then
  echo ""
else
  echo "wrong parameter count"
  echo "fw_update.sh <port> <model name|commXEL or powerXEL or sensorXEL>"
  exit
fi

case $2 in
  [c][o][m][m][X][E][L]) $shell_cmd $1 57600 0x08040000 ../fw_binaries/commXEL_app.binary 1 1
  ;;
  [p][o][w][e][r][X][E][L]) $shell_cmd $1 57600 0x08005000 ../fw_binaries/powerXEL_app.binary 1 1
e
  ;;
  [s][e][n][s][o][r][X][E][L]) $shell_cmd $1 57600 0x08005000 ../fw_binaries/sensorXEL_app.binary 1 1
  ;;
  *)
  echo "wrong model parameter"
  echo "fw_update.sh <port> <model name|commXEL or powerXEL or sensorXEL>"
  ;;
esac

exit
