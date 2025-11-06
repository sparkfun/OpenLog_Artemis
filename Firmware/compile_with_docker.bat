docker build -t openlog_artemis_firmware --progress=plain --no-cache-filter deployment .
docker create --name=openlog_artemis_container openlog_artemis_firmware:latest
docker cp openlog_artemis_container:/OpenLog_Artemis.ino.bin .
docker container rm openlog_artemis_container
