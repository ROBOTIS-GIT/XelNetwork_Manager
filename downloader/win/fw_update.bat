@echo off

echo "XEL Firmware Update Start.."

set argc=0
for %%x in (%*) do Set /A argc+=1

if %argc%==2 (
  echo ""
) else (
  echo "wrong parameter count"
  echo "fw_update.sh <port> <model name|commXEL or powerXEL or sensorXEL>"
  GOTO:EOF
)

if "%2"=="commXEL" (
  xel_loader.exe %1 57600 0x08040000 ../fw_binaries/commXEL_app.binary 1 1 
  GOTO:EOF
)

if "%2"=="powerXEL" (
  xel_loader.exe %1 57600 0x08005000 ../fw_binaries/powerXEL_app.binary 1 1
  GOTO:EOF
)

if "%2"=="sensorXEL" (
  xel_loader.exe %1 57600 0x08005000 ../fw_binaries/sensorXEL_app.binary 1 1
  GOTO:EOF
)

echo "wrong model parameter"
echo "fw_update.sh <port> <model name|commXEL or powerXEL or sensorXEL>"

GOTO:EOF