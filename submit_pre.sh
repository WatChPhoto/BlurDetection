#!/bin/bash

function submit {
    echo sbatch ${SUBFILE}
    sbatch ${SUBFILE}
}

WORKDIR=${PWD}

BASEDIR=${WORKDIR}/output

LOGDIR=${BASEDIR}/log
mkdir -p ${LOGDIR}

SCRIPTDIR=${BASEDIR}/script
mkdir -p ${SCRIPTDIR}

DIRS=(BarrelSurveyFar  BarrelSurveyRings  BottomCornerSurvey  BottomSurvey  TopCornerSurvey  TopSurvey)
#DIRS=(BottomSurvey)

rootpath=/project/rpp-blairt2k/public_html/TOW-Feb2020/Surveys

for dir in ${DIRS[@]}; do

    path=${rootpath}/${dir}
    
    outdir=${BASEDIR}/${dir}
    mkdir -p ${outdir}

    burst_root=0
    nBursts=-1

    for file in ${path}/B*; do
	filename=$(basename -- "$file")

        current_burst_root=${filename:1:3}
        #echo ${file} $current_burst_root

	nBursts=$(( nBursts + 1 ))
	#echo $nBursts

        #if [[ ${burst_root} == 0 ]]; then
        #    burst_root=${current_burst_root}
        #    continue
        #fi

	if [[ ${current_burst_root} != ${burst_root} ]]; then

            basename=${dir}_${current_burst_root}

	    SUBFILE=${SCRIPTDIR}/${basename}.sh
	    cat > ${SUBFILE}<<EOF
#!/bin/bash

#SBATCH --job-name=${basename}

#SBATCH --cpus-per-task=1
#SBATCH --mem=4G        
#SBATCH --time=0-00:30 

#SBATCH --account=rpp-blairt2k

#SBATCH --output=${LOGDIR}/${basename}.log
#SBATCH --error=${LOGDIR}/${basename}.log

cd ${WORKDIR}

source setup.sh

time ./BlurDetection.py -o ${outdir}/B${current_burst_root}.txt -i "${path}/B${current_burst_root}*"

EOF
            if [[ ${nBursts} != 10 ]]; then
                echo "Warning: nBursts = ${nBursts}: ${burst_root}"
            fi

            burst_root=${current_burst_root}
            submit
            nBursts=0

        fi

    done

    if ! ls ${path}/SING* 1> /dev/null 2>&1; then
	continue 
    fi


    nImages=0

    for file in ${path}/SING*; do

        filename=$(basename -- "$file")
	fileroot=${filename%.*}
        basename=${dir}_${fileroot}

        if [[ ${nImages} == 10 ]]; then
            submit
	    nImages=0
	fi

        if [[ ${nImages} == 0 ]]; then

  	    SUBFILE=${SCRIPTDIR}/${basename}.sh
            cat > ${SUBFILE}<<EOF
#!/bin/bash

#SBATCH --job-name=${basename}

#SBATCH --cpus-per-task=1
#SBATCH --mem=4G        
#SBATCH --time=0-00:30 

#SBATCH --account=rpp-blairt2k

#SBATCH --output=${LOGDIR}/${basename}.log
#SBATCH --error=${LOGDIR}/${basename}.log

cd ${WORKDIR}

source setup.sh

EOF
	fi

        cat >> ${SUBFILE}<<EOF
time ./BlurDetection.py -o ${outdir}/${fileroot}.txt -i ${file}
EOF
	nImages=$(( nImages + 1 ))

    done
    submit
done
