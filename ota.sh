#!/bin/bash
scp build/iot.bin root@srv3.enteam.pl:/root/iot/firmware/iot.bin
scp version.json root@srv3.enteam.pl:/root/iot/firmware/version.json

# build project
# bump version in version.json
# ./ota.sh
# wait