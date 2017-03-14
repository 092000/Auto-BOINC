#!/bin/bash

PJroot=/home/boincadm/projects/test
AppName=$1
AppFiles=$2

# check existence of those files

if [ "${AppName}" == "" ] || ! [ -e "${AppFiles}/${AppName}" ] ; then
        echo "Error : Lack of the App !"
        exit 1
fi

if ! [ -e "${AppFiles}/${AppName}_in" ] || ! [ -e "${AppFiles}/${AppName}_out" ] ; then
	echo "Error : Lack of Temmplates !"
	exit 2
fi

if ! [ -d "${AppFiles}/inputfiles" ]; then
	echo "Error : Lack of dir ${AppFiles}/inputfiles, All input data should be here !"
	exit 3
fi

if ! [ -e "${AppFiles}/inputfiles/rules" ] ; then
        echo "Error : Lack of rules for input and output files !"
	echo -e "You should give rules as follow and re-run this script :\n"
	echo "	X file_in_1 file_in_2 file_in_3 ...... file_in_X"
	echo -e "	Y file_out_1 file_out_2 file_out_3 ...... file_out_Y\n"
        exit 4
fi

if [ -d "${PJroot}/apps/${AppName}" ]; then
	echo "Error : This APP already exits"
	exit 5
fi


# config for new app

cd ${PJroot}

line=$(wc -l config.xml | cut -d ' ' -f 1)
head -n $((line-2)) config.xml > tmpf
mv tmpf config.xml
echo -e "    <daemon>\n      <cmd>sample_work_generator_${AppName} ${AppFiles}/inputfiles --app ${AppName} --in_template_file ${AppName}_in --out_template_file ${AppName}_out</cmd>\n    </daemon>" >> config.xml
echo -e "    <daemon>\n      <cmd>sample_trivial_validator_${AppName} -d 3 --app ${AppName}</cmd>\n    </daemon>" >> config.xml
echo -e "    <daemon>\n      <cmd>sample_assimilator_${AppName} -d 3 --app ${AppName}</cmd>\n    </daemon>" >> config.xml

echo -e "  </daemons>\n</boinc>" >> config.xml

line=$(wc -l project.xml | cut -d ' ' -f 1)
head -n $((line-1)) project.xml > tmpf
mv tmpf project.xml
echo -e "    <app>\n        <name>${AppName}</name>\n        <user_friendly_name>${AppName}</user_friendly_name>\n    </app>" >> project.xml
echo -e "</boinc>" >> project.xml

cp "${AppFiles}/${AppName}_in" "${PJroot}/templates"
cp "${AppFiles}/${AppName}_out" "${PJroot}/templates"
cp /home/boincadm/boinc-master/sched/sample_work_generator "${PJroot}/bin/sample_work_generator_${AppName}"
cp /home/boincadm/boinc-master/sched/sample_trivial_validator "${PJroot}/bin/sample_trivial_validator_${AppName}"
cp /home/boincadm/boinc-master/sched/sample_assimilator "${PJroot}/bin/sample_assimilator_${AppName}"

mkdir "${PJroot}/apps/${AppName}"
mkdir "${PJroot}/apps/${AppName}/1.0"
mkdir "${PJroot}/apps/${AppName}/1.0/x86_64-pc-linux-gnu"
cd "${PJroot}/apps/${AppName}/1.0/x86_64-pc-linux-gnu/"
cp /home/boincadm/boinc-master/samples/example_app/uc2 ./uc2-${AppName}
cp "${AppFiles}/${AppName}" ./
cat /home/boincadm/projects/version.xml | sed "s/a.out/${AppName}/" | sed "s/uc2/uc2-${AppName}/"  >> version.xml

cd ${PJroot}
./bin/xadd
./bin/update_versions
./bin/start

