#!/bin/bash

DIR=/home/boincadm/ProgramData/input
LOG=/home/boincadm/projects/test/log_debian7/GrabAPPs.log
APPS=/home/boincadm/projects/test/apps
flag=0

printf "\nstarted at %s %s \n" $(date "+%Y-%m-%d %H:%M:%S") >> ${LOG}

while (true); do

#validate results and assimilate them below
	if [ -d  ${APPS} ]; then
		cd ${APPS}/../
		for appi in $(ls ${APPS} -l | grep "^d" | awk '{print $NF}')
		do
			./bin/sample_trivial_validator --app ${appi} &> /dev/null
			./bin/sample_assimilator --app ${appi} &> /dev/null
		done
		cd - &> /dev/null
	fi

#scan new app below
        retavl=$(ls ${DIR} | grep "^OK$" | wc -l)
        if ((retavl==0)); then
		if [ "$flag" == "0" ]; then
			flag=1
			 printf "\nsleep (%s %s) \n" $(date "+%Y-%m-%d %H:%M:%S") >> ${LOG}
		fi
                sleep 30
        else
		flag=0
		rm ${DIR}/OK
		printf "find [OK] at (%s %s) \n" $(date "+%Y-%m-%d %H:%M:%S") >> ${LOG}
		for var in $(ls ${DIR} -l | grep "^d" | awk '{print $NF}')
		do
			printf "\t test for project %s ... " ${var} >> ${LOG}
			if [ -f ${DIR}/${var}/boinc.config ]; then
				 dos2unix ${DIR}/${var}/boinc.config
			fi
			check_msg=$(check_app ${var})
			printf "%s\n" ${check_msg} >> ${LOG}
			if ! [[ ${check_msg} == ERROR* ]]; then
				printf "%s data case are found. \n" ${cnt} >> ${LOG}
				NewappForWindows ${var} ${DIR}/${var}
				if [ -d ${DIR}/../submitted/${var} ]; then
					rm -r ${DIR}/../submitted/${var}
				fi
				mv ${DIR}/${var} ${DIR}/../submitted/
			fi
		done
        fi
done


