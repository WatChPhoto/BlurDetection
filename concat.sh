DIRS=(BarrelSurveyFar  BarrelSurveyRings  BottomCornerSurvey  BottomSurvey  TopCornerSurvey  TopSurvey)

for dir in ${DIRS[@]}; do
    cat output/${dir}/* > output/${dir}.txt &
done
