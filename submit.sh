#!/bin/bash

WORKDIR=${PWD}

BASEDIR=${WORKDIR}/output

LOGDIR=${BASEDIR}/log
mkdir -p ${LOGDIR}

SCRIPTDIR=${BASEDIR}/script
mkdir -p ${SCRIPTDIR}

DIRS=(BarrelSurveyFar_TB3  BarrelSurveyRings_TB3  BottomCornerSurvey_TB3  BottomSurvey_TB3  TopCornerSurvey_TB3  TopSurvey_TB3)
CONFIGS=(images blurry veryblurry)

rootpath=/project/rpp-blairt2k/public_html/TOW-Feb2020/Processed

for dir in ${DIRS[@]}; do
    for config in ${CONFIGS[@]}; do

        path=${rootpath}/${dir}/*/${config}

        basename=${dir}_${config}
	SUBFILE=${SCRIPTDIR}/${basename}.sh
	cat > ${SUBFILE}<<EOF
#!/bin/bash

#SBATCH --job-name=${basename}

#SBATCH --cpus-per-task=32         # CPU cores/threads
#SBATCH --mem=120G              # memory per node
#SBATCH --time=0-00:30            # time (DD-HH:MM)

#SBATCH --account=rpp-blairt2k

#SBATCH --output=${LOGDIR}/${basename}.log
#SBATCH --error=${LOGDIR}/${basename}.log

cd ${WORKDIR}

source setup.sh

time ipython -c "%run BlurDetection.py -o ${BASEDIR}/${dir}_${config}.txt -i \"${path}/\*.jpg\""

EOF

        sbatch ${SUBFILE}

    done
done
