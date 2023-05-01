DIRS=(BarrelSurveyFar  BarrelSurveyRings  BottomCornerSurvey  BottomSurvey  TopCornerSurvey  TopSurvey)

for dir in ${DIRS[@]}; do
    (
      cat output/${dir}/* > output/${dir}_unsrt.txt 
      sort -k1 -n output/${dir}_unsrt.txt > output/${dir}.txt
      rm output/${dir}_unsrt.txt
    )&
done
