#!/bin/bash

source setup.sh

DIRS=(BarrelSurveyFar_TB3  BarrelSurveyRings_TB3  BottomCornerSurvey_TB3  BottomSurvey_TB3  TopCornerSurvey_TB3  TopSurvey_TB3)
CONFIGS=(images blurry veryblurry)

rootpath=/project/rpp-blairt2k/public_html/TOW-Feb2020/Processed

for dir in ${DIRS[@]}; do
    for config in ${CONFIGS[@]}; do

        path=${rootpath}/${dir}/*/${config}

	time ipython -c "%run BlurDetection.py -o ${dir}_${config}.txt -i \"${path}/\*.jpg\""
    done
done
