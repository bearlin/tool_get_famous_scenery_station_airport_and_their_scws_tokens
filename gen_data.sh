#!/bin/bash

# Make sure python is there
HasPython=0
which python &> /dev/null
if [ $? -eq 0 ]; then
  HasPython=1
fi
if [ $HasPython == 0 ] ; then
  echo -e "\t=======\t"
  echo -e "\tpython commands not found, please install python first."
  echo -e "\tByeBye!\t"
  echo -e "\t=======\t"
  echo
  exit 0
fi

echo -e "python commands found!"
printf "Version info : "
python --version

cd inputs/airports
python extract_china_airports_raw.py china_airports_raw.txt
python extract_taiwan_airports_raw.py taiwan_airports_raw.txt
cd -

cd inputs/scenery
python extract_china_scenery_raw.py china_scenery_raw.txt
python extract_taiwan_scenery_raw.py taiwan_scenery_raw.txt
cd -

cd inputs/stations
python extract_china_station_raw.py china_stations_raw.txt
python extract_taiwan_station_raw.py taiwan_stations_raw.txt
cd -

