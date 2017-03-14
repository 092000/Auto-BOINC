
#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>

#include "backend_lib.h"
#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "svn_version.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define FILE_NUM 1024
#define CUSHION 199999999
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  1
    // number of instances of each job

using std::string;
using std::map;

const char* app_name = "example_app";
const char* in_template_file = "example_app_in";
const char* out_template_file = "example_app_out";

char* in_template;
char INPUT_ROOT[256];
DB_APP app;
int start_time;
int seqno;
int single_case;

//report error log
void REPORT_ERR(const char* S_ERR){
	FILE* f_MSG = fopen("/home/boincadm/projects/test/log_debian7/run_MSG", "a");
        fprintf(f_MSG, "Error  from work generator : %s !\n", S_ERR);
        fclose(f_MSG);
}

//copy file from one to another
void fccpp(char* S1, char* S2){
	char tmps[1024];
	int cnt;
	FILE* f1 = fopen(S1, "rb");
	FILE* f2 = fopen(S2, "wb");
	if (!f1) {
		REPORT_ERR("Still can't find file.");
		return ;
	}
	while ( (cnt = fread(tmps, 1, 1024, f1)) > 0)
		fwrite(tmps, 1, cnt, f2);
	fclose(f1); fclose(f2);
}

// create one new job according to my format
int make_job() {
    DB_WORKUNIT wu;
    char name[256], names[FILE_NUM][256], Rnames[FILE_NUM][256], path[MAXPATHLEN];
	char myfiles[256];
    const char *infiles[FILE_NUM];
    int retval, INPUT_NUM;

    // make a unique name (for the job and its input file)
    //
	sprintf(Rnames[0], "%s/boincs.config", INPUT_ROOT);

	FILE* fp_rules = fopen(Rnames[0], "r");
	if (!fp_rules) {
		// REPORT_ERR(Rnames[0]);
		REPORT_ERR("RULES file doesn't exit !");
		return 1;
	}
	sprintf(Rnames[0], "boincs.config");
	fscanf(fp_rules, "%d", &INPUT_NUM);
	for (int i=1; i<=INPUT_NUM; i++) 
		fscanf(fp_rules, "%s", Rnames[i]);

	sprintf(name, "%s_%d", app_name, ++seqno);
	for (int i=0; i<=INPUT_NUM; i++)
		sprintf(names[i], "%d_%s_%s", seqno, app_name, Rnames[i]);
	fclose(fp_rules);
	if (seqno == 1) single_case = 0;
	if (seqno == 2 && single_case == INPUT_NUM)
		return -1;
	for (int i=1; i<=INPUT_NUM; i++){
		if (seqno == 1){
			sprintf(myfiles, "%s/%s", INPUT_ROOT, Rnames[i]);
			fp_rules = fopen(myfiles, "rb");
			if (fp_rules){
				single_case++;
				fclose(fp_rules);
			}			
		}
		sprintf(myfiles, "%s/%d_%s", INPUT_ROOT, seqno, Rnames[i]);
		fp_rules = fopen(myfiles, "rb");
		if (fp_rules) {
			fclose(fp_rules);
			continue;
		} else {
			sprintf(myfiles, "%s/%s", INPUT_ROOT, Rnames[i]);
			fp_rules = fopen(myfiles, "rb");
			if (fp_rules)
				fclose(fp_rules);
			else return -1;
		}
	}


    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //

	for (int i=1; i<=INPUT_NUM; i++) {
		retval = config.download_path(names[i], path);
		if (retval) return retval;
		sprintf(myfiles, "%s/%d_%s", INPUT_ROOT, seqno, Rnames[i]);
		fp_rules = fopen(myfiles, "rb");
		if (!fp_rules) 
			sprintf(myfiles, "%s/%s", INPUT_ROOT, Rnames[i]);
		fccpp(myfiles, path);
	}

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    safe_strcpy(wu.name, name);
	safe_strcpy(wu.app_name, app_name);
    wu.rsc_fpops_est = 1e12;
    wu.rsc_fpops_bound = 1e14;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 86400;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
	// infiles[1] = names[1];
	for (int i=0; i<=INPUT_NUM; i++)
		infiles[i] = names[i];

    // Register the job with BOINC
    //
    sprintf(path, "templates/%s", out_template_file);
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles+1,
        INPUT_NUM,
        config
    );
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n, app.id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "count_unsent_jobs() failed: %s\n", boincerror(retval)
            );
            exit(retval);
        }
        if (n > CUSHION) {
            daemon_sleep(10);
        } else {
            int njobs = (CUSHION-n)/REPLICATION_FACTOR;
            log_messages.printf(MSG_DEBUG,
                "Making %d jobs\n", njobs
            );
            for (int i=0; i<njobs; i++) {
                retval = make_job();
		if (retval == -1) return ;
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "can't make job: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
            }
            // Wait for the transitioner to create instances
            // of the jobs we just created.
            // Otherwise we'll create too many jobs.
            //
            double now = dtime();
            while (1) {
                daemon_sleep(5);
                double x;
                retval = min_transition_time(x);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "min_transition_time failed: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
                if (x > now) break;
            }
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

	if (argc < 2) {
		REPORT_ERR("too few argc !");
		return 1;
	}
	strcpy(INPUT_ROOT, argv[1]);

    for (i=2; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
	return 0;
}
