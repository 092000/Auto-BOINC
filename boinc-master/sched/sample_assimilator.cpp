// A sample assimilator that:
// 1) if success, copy the output file(s) to a directory
// 2) if failure, append a message to an error log

#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"

using std::vector;
using std::string;

int outnum, casenum;
char outf[1024][1024];
bool outflag;

//write error logs
void REPORT_ERR(const char* S_ERR){
        FILE* f_MSG = fopen("/home/boincadm/projects/test/log_debian7/run_MSG", "a");
        fprintf(f_MSG, "Error  from assimilator : %s !\n", S_ERR);
        fclose(f_MSG);
}

//to know the format
void handle_outf(const char* app_name){
	outflag = true;
	char config_path[1024];
	sprintf(config_path, "/home/boincadm/ProgramData/submitted/%s/inputfiles/boincs.config", app_name);
	FILE* FCONFIG = fopen(config_path, "rb");
	if (!FCONFIG){
		REPORT_ERR("no config file found in ProgramData/submitted");
		outflag = false;
		return ;
	}
	fscanf(FCONFIG, "%d", &outnum);
	while (outnum--) fscanf(FCONFIG, "%s", outf[0]);
	fscanf(FCONFIG, "%d", &outnum);
	for (int i=0; i<outnum; i++)
		fscanf(FCONFIG, "%s", outf[i]);
    fclose(FCONFIG);
}

//error occured
int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen(config.project_path("download/00000000/errors"), "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    int retval;
    char buf[1024], app_name[256];
    unsigned int i;

	for (i=strlen(wu.name)-1; wu.name[i]!='_'; i--)
		if (i==0) break;
	if (i==0) {
		REPORT_ERR("wu.name may not contain '_' or it is illegal ");
		return -1;
	}
	memset(app_name, 0, sizeof(app_name));
	for (unsigned j=0; j<i; j++) app_name[j]=wu.name[j];
    //all results be put in download/00000000
    retval = boinc_mkdir(config.project_path("download/00000000"));
    retval |= boinc_mkdir(config.project_path("download/00000000/%s", app_name));
    if (retval) return retval;

	handle_outf(app_name);

    if (wu.canonical_resultid) {
        vector<OUTPUT_FILE_INFO> output_files;
        const char *copy_path;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
	if ((int)n != outnum){
		REPORT_ERR("n is not equal to outnum");
		outflag = false;
	}
	casenum = 0;
	int mult = 1;
	for (int j=strlen(wu.name)-1; j>=0; j--){
		if (wu.name[j] != '_'){
			casenum += (int)(wu.name[j] - '0') * mult;
			mult *= 10;
		}
		else break;
	}
	if (casenum == 0){
		REPORT_ERR("casenum = 0");
		outflag = false;
	}
        bool file_copied = false;
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
		if (outflag)
	                copy_path = config.project_path("download/00000000/%s/%d_%s", app_name, casenum, outf[i]);
		else 
	                copy_path = config.project_path("download/00000000/%s/%s", app_name, wu.name);

            retval = boinc_copy(fi.path.c_str() , copy_path);
            if (!retval) {
                file_copied = true;
            }
        }
        if (!file_copied) {
            copy_path = config.project_path(
                "download/00000000/%s/%s_%s", app_name, wu.name, "no_output_files"
            );
            FILE* f = fopen(copy_path, "w");
            fclose(f);
        }
    } else {
        sprintf(buf, "%s: 0x%x\n", wu.name, wu.error_mask);
        return write_error(buf);
    }
    return 0;
}
