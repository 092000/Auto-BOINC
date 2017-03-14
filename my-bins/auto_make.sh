#!/bin/bash

echo ""
read -p "New Project Name > " ProjectName
read -p "Load test_app ?(y/n) > " Test_app
echo ""

if [ $Test_app == "y" ]; then
	Test_app="--test_app"
else 
	Test_app=""
fi
	
cd /home/boincadm/boinc-master/tools
./make_project  --delete_prev_inst --drop_db_first --url_base http://202.197.61.226:5002/ ${Test_app} ${ProjectName}

cd /home/boincadm/projects/${ProjectName}
su -c "cat ${ProjectName}.httpd.conf > /etc/apache2/conf.d/${ProjectName}.httpd.conf"
su -c "apache2ctl -k restart"
crontab ${ProjectName}.cronjob
htpasswd -cb ~/projects/${ProjectName}/html/ops/.htpasswd boincadm boincadmpw

 cp /home/boincadm/projects/my-bins/config.xml_Windows /home/boincadm/projects/test/config.xml
 cp /home/boincadm/projects/my-bins/project.xml_Windows /home/boincadm/projects/test/project.xml
 cp /home/boincadm/saves/update_versions-NOASK /home/boincadm/projects/test/bin/update_versions
 cp /home/boincadm/saves/config_aux.xml /home/boincadm/projects/test/


cd /home/boincadm/projects/${ProjectName}
 ./bin/xadd
 ./bin/update_versions
 ./bin/start

