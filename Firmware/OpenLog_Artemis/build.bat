
mkdir build
arduino-cli.exe compile -v --output-dir . -b SparkFun:apollo3:sfe_artemis_atp --build-cache-path build --build-property compiler.cpp.extra_flags="-DICM_20948_USE_DMP" OpenLog_Artemis.ino 