#!/bin/bash


unzip src.zip
cp -r src $1


gcc src/master/master.c -pthread -o master -lm

gcc src/drone_yh11/drone_yh11.c -o drone_yh11

gcc src/drone_awais/drone_awais.c -o drone_awais

gcc src/drone_bm3/drone_bm3.c -o drone_bm3

gcc src/drone_ha1/drone_ha1.c -lpthread -o drone_ha1 -lm

gcc src/drone_ja1/drone_ja1.c -lm -o drone_ja1





