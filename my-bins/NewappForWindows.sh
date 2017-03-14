#!/bin/bash

PJroot=/home/boincadm/projects/test
AppName=$1
AppFiles=$2

# check existence of those files


if [ -d "${PJroot}/apps/${AppName}" ]; then
	echo "Error : This APP already exits"
	exit 5
fi


# config for new app

cd ${PJroot}

line=$(wc -l project.xml | cut -d ' ' -f 1)
head -n $((line-1)) project.xml > tmpf
mv tmpf project.xml
echo -e "    <app>\n        <name>${AppName}</name>\n        <user_friendly_name>${AppName}</user_friendly_name>\n    </app>" >> project.xml
echo -e "</boinc>" >> project.xml

cd ${AppFiles}
make_templates ${AppName}
cp "${AppFiles}/${AppName}_in" "${PJroot}/templates"
cp "${AppFiles}/${AppName}_out" "${PJroot}/templates"

mkdir "${PJroot}/apps/${AppName}"
mkdir "${PJroot}/apps/${AppName}/1.0"
mkdir "${PJroot}/apps/${AppName}/1.0/windows_x86_64"
cd "${PJroot}/apps/${AppName}/1.0/windows_x86_64"
cp /home/boincadm/projects/my-bins/wrapper_26016_windows_x86_64.exe ./wrapper_${AppName}.exe
cp /home/boincadm/projects/my-bins/uc2w.exe ./uc2w_${AppName}.exe
cp "${AppFiles}/${AppName}" ./

echo -e "<job_desc>\n    <task>\n        <application>WORKER</application>\n        <time_limit>43222</time_limit>\n    </task>\n</job_desc>" |sed "s/WORKER/uc2w_${AppName}.exe/" > ${AppName}_job_1.0.xml

echo -e "<version>\n   <file>\n      <physical_name>wrapper_${AppName}.exe</physical_name>\n      <main_program/>\n   </file>\n   <file>\n      <physical_name>uc2w_${AppName}.exe</physical_name>\n      <logical_name>uc2w_${AppName}.exe</logical_name>\n   </file>\n   <file>\n      <physical_name>${AppName}</physical_name>\n      <logical_name>${AppName}</logical_name>\n   </file>\n   <file>\n      <physical_name>${AppName}_job_1.0.xml</physical_name>\n      <logical_name>job.xml</logical_name>\n   </file>\n</version>" > version.xml

 cd ${PJroot}
 ./bin/xadd
 ./bin/update_versions
 ./bin/start

./bin/sample_work_generator ${AppFiles}/inputfiles --app ${AppName} --in_template_file ${AppName}_in --out_template_file ${AppName}_out

