#include <cstdio>

char tempNAME[1024], fname[1024];
int INd, OUTd;
FILE* config = fopen("inputfiles/boincs.config", "rb");

void make_in_temp(const char* app_name) {
	sprintf(tempNAME, "%s_in", app_name);
	FILE* INtemp = fopen(tempNAME, "wb");
	fscanf(config, "%d", &INd);
	for (int i=0; i<INd; i++)
		fprintf(INtemp, "<file_info>\n\t<number>%d</number>\n</file_info>\n", i);
	fprintf(INtemp, "<workunit>\n");
	for (int i=0; i<INd; i++) {
		fscanf(config, "%s", fname);
		fprintf(INtemp, "\t<file_ref>\n\t\t<file_number>%d</file_number>\n\t\t<open_name>%s</open_name>\n\t\t<copy_file/>\n\t</file_ref>\n", i, fname);
	}
	fprintf(INtemp, "</workunit>\n");
	fclose(INtemp);
}

void make_out_temp(const char* app_name) {
	sprintf(tempNAME, "%s_out", app_name);
	FILE* OUTtemp = fopen(tempNAME, "wb");
	fscanf(config, "%d", &OUTd);
	for (int i=0; i<OUTd; i++)
		fprintf(OUTtemp, "<file_info>\n\t<name><OUTFILE_%d/></name>\n\t<generated_locally/>\n\t<upload_when_present/>\n\t<max_nbytes>5000000</max_nbytes>\n\t<url><UPLOAD_URL/></url>\n</file_info>\n", i);
	fprintf(OUTtemp, "<result>\n");
	for (int i=0; i<OUTd; i++) {
		fscanf(config, "%s", fname);
		fprintf(OUTtemp, "\t<file_ref>\n\t\t<file_name><OUTFILE_%d/></file_name>\n\t\t<open_name>%s</open_name>\n\t\t<copy_file/>\n\t</file_ref>\n", i, fname);
	}
	fprintf(OUTtemp, "</result>\n");
	fclose(OUTtemp);
}

int main(int argc, char** argv){
	if (argc != 2) {
		printf("ERROR while making templates, lack of ARGV \n");
	}
	make_in_temp(argv[1]);
	make_out_temp(argv[1]);
	fclose(config);
	return 0;
}

