#include <cstdio>
#include <cstring>

char cmdfile[1024], appname[1024], appfiles[1024], fbat[1024], rules[1024], tmp[10240], thecmd[1024];
char output[64][1024];
int out_num;

int main(int argc, char** argv){ // AppName AppFiles 
	sprintf(appname, "%s", argv[1]);
	sprintf(appfiles, "%s", argv[2]);
	sprintf(cmdfile, "%s/inputfiles/cmd", appfiles);
	sprintf(rules, "%s/inputfiles/rules", appfiles);
	sprintf(fbat, "%s/%s.bat", appfiles, appname);
	FILE* FCMD = fopen(cmdfile, "rb");
	FILE* FBAT = fopen(fbat, "wb");
	FILE* FRULES = fopen(rules, "rb");
	if (!FCMD) {
		fprintf(FBAT, "for /d %%%%i in (..\\..\\projects\\*_test) do call %%%%i\\%s\n", appname);
	} else {
		fscanf(FRULES, "%d", &out_num);
		while (out_num--) fscanf(FRULES, "%s", tmp);
		fscanf(FRULES, "%d", &out_num);
		for (int i=1; i<=out_num; i++){
			fscanf(FRULES, "%s", output[i]);
			fprintf(FBAT, "echo.>%s_tmp\n", output[i]);
		}
		fprintf(FBAT, "for /d %%%%i in (..\\..\\projects\\*_test) do ( \n");
		while (fgets(thecmd, 1024, FCMD)){
			fprintf(FBAT, "\tcall %%%%i\\%s %s ", appname, thecmd);
			for (int i=1; i<=out_num; i++)
				fprintf(FBAT, "\ttype %s >> %s_tmp \n\tdel %s \n\n", output[i], output[i], output[i]);
		}
		fprintf(FBAT, ")\n\n");
		for (int i=1; i<=out_num; i++)
			fprintf(FBAT, "type %s_tmp > %s \ndel %s_tmp \n", output[i], output[i], output[i]);
		
	}
	fclose(FRULES);
	fclose(FBAT);
	if (FCMD) fclose(FCMD);
}
