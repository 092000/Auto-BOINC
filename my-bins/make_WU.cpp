#include <cstdio>
#include <cstring>
#include <cstdlib>

char app_name[1024], pwd[1024];
char fname[50][1024];
int fnum, seqno=1;

bool MKW() {
	char tmps[1024], buf[1024], tmp_cmd[1024];
	FILE* tmpf;
	for (int i=1; i<=fnum; i++){
		sprintf(tmps, "inputfiles/%s_%d", fname[i], seqno);
		tmpf = fopen(tmps, "rb");
		if (!tmpf) return false;
		fclose(tmpf);
	}

	char CMD1[10240], CMD2[10240];
	sprintf(CMD2, "cd /home/boincadm/projects/test && ./bin/create_work --appname %s --wu_name %s_WU_%d ", app_name, app_name, seqno);

	for (int i=1; i<=fnum; i++){
		sprintf(tmp_cmd, "cp inputfiles/%s_%d inputfiles/%s_%s_%d", fname[i], seqno, app_name, fname[i], seqno);
		system(tmp_cmd);
		sprintf(CMD1, "cd /home/boincadm/projects/test && ./bin/stage_file %s/inputfiles/%s_%s_%d && cd - ", pwd, app_name, fname[i], seqno);
		printf("\tCMD1=%s\n", CMD1);
		system(CMD1);
		sprintf(buf, "%s_%s_%d ", app_name, fname[i], seqno);
		strcat(CMD2, buf);
	}
	printf("\tcmd2= %s\n", CMD2);
	system(CMD2);
	seqno++;
	return true;
}

int main(int argc, char** argv) {
	if (argc<1) return -1;
	strcpy(app_name, argv[1]);
	FILE* FRULES = fopen("inputfiles/rules", "rb");
	fscanf(FRULES, "%d", &fnum);
	printf("number of inputfiles for each case = %d \napp_name = %s\n", fnum, app_name);
	for (int i=1; i<=fnum; i++) 
		fscanf(FRULES, "%s", fname[i]);
	fclose(FRULES);
	FILE* PWD = popen("pwd ", "r");
	fgets(pwd, sizeof(pwd), PWD);
	pclose(PWD);
	if (pwd[strlen(pwd)-1]=='\n') pwd[strlen(pwd)-1]='\0';
	while (1)
		if (!MKW()) break;
	return 0;
}

