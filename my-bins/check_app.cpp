#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;

char app[1024],root[1024];
char infile[1024][1024], outfile[1024][1024];
int N_in, N_out;
FILE* config;
bool cmdlist;

bool check_exist(const char* S1){
    FILE* f_tmp = fopen(S1, "rb");
    if (!f_tmp) return false;
        else return true;
}

bool check_exist(const char* S1, const char* S2){
    char s_tmp[1024];
    sprintf(s_tmp, "%s%s", S1, S2);
    return check_exist(s_tmp);
}
bool check_exist(const char* S1, const char* S2, const char* S3){
    char s_tmp[1024];
    sprintf(s_tmp, "%s%s%s", S1, S2, S3);
    return check_exist(s_tmp);
}

bool check_config(){
    N_in = N_out = 0;
    memset(infile, 0, sizeof(infile));
    memset(outfile, 0, sizeof(outfile));
    int cnt_in=0, cnt_out=0;
    char config_root[1024], tmps[1024];
    sprintf(config_root, "%s%s", root, "boinc.config");
    config = fopen(config_root, "rb");
    fscanf(config, "%d", &N_in);
    fgets(tmps, 1024, config);
    if (tmps[strlen(tmps)-1] == '\n') tmps[strlen(tmps)-1] = '\0';
    char* pch = strtok(tmps, " ");
    while (pch != NULL) {
        cnt_in++;
        strcpy(infile[cnt_in], pch);
        pch = strtok(NULL, " ");
    }
    if (cnt_in != N_in) return false;
    fscanf(config, "%d", &N_out);
    fgets(tmps, 1024, config);
    if (tmps[strlen(tmps)-1] == '\n') tmps[strlen(tmps)-1] = '\0';
    pch = strtok(tmps, " ");
    while (pch != NULL) {
        cnt_out++;
        strcpy(outfile[cnt_out], pch);
        pch = strtok(NULL, " ");
    }
    if (cnt_out != N_out) return false;
    fclose(config);
    return true;
}

bool check_inputfile(const char* filename){
    return check_exist(root, "inputfiles/", filename);
}

bool check_inputfile(const char* filename, const int casenum){
    char s_tmp[1024];
    sprintf(s_tmp, "inputfiles/%d_", casenum);
    return check_exist(root, s_tmp, filename);
}

void boincapp(){
    char s_tmp[1024];
    sprintf(s_tmp, "%sinputfiles/boincapp.config", root);
    config = fopen(s_tmp, "wb");
    fprintf(config, "%s\n\n\n", app);
    fclose(config);
}

void boincapp(int cnt, const char* thecmd){
    char s_tmp[1024];
    sprintf(s_tmp, "%sinputfiles/%d_boincapp.config", root, cnt);
    config = fopen(s_tmp, "wb");
    fprintf(config, "%s\n%s\n\n", app, thecmd);
    fclose(config);
}

void rebuild_config(){
    char s_tmp[1024];
    sprintf(s_tmp, "%sinputfiles/boincs.config", root);
    config = fopen(s_tmp, "wb");
    fprintf(config, "%d ",N_in+1);
    for (int i=1; i<=N_in; i++)
        fprintf(config, "%s ", infile[i]);
    fprintf(config, "boincapp.config \n");
    fprintf(config, "%d ",N_out);
    for (int i=1; i<=N_out; i++)
        fprintf(config, "%s ", outfile[i]);
    fprintf(config, "\n");
    fclose(config);
}

int main(int argc, char** argv){ // appname = argv[1]
    strcpy(app, argv[1]);
    sprintf(root, "/home/boincadm/ProgramData/input/%s/", app);
    if (!check_exist(root, app)) {
        printf("ERROR:no app found");
        return 0;
    }
    if (!check_exist(root, "boinc.config")){
        printf("ERROR:no boinc.config");
        return 0;
    }
    if (!check_config()){
        printf("ERROR:config format error");
        return 0;
    }
    /*
    cout<<N_in<<endl;
    for (int i=1; i<=N_in; i++) cout<<infile[i]<<endl;
    cout<<N_out<<endl;
    for (int i=1; i<=N_out; i++) cout<<outfile[i]<<endl;
    */
    if (check_exist(root, "inputfiles/cmd.list"))
        cmdlist = true;
    else
        cmdlist = false;

    bool has_boincapp = false;
    for (int i=1; i<=N_in; i++)
        if (strcmp(infile[i], "boincapp.config") == 0)
            has_boincapp = true;

    if (cmdlist){
        for (int i=1; i<=N_in; i++){
            if (!check_inputfile(infile[i])) {
                printf("ERROR:incomplete input files");
                return 0;
            }
        }
        char s_tmp[1024];
        int cnt = 0;
        sprintf(s_tmp, "%sinputfiles/cmd.list", root);
        FILE* cmdfile = fopen(s_tmp, "rb");
        while (fgets(s_tmp, 1024, cmdfile)) {
            cnt++;
            if (!has_boincapp) boincapp(cnt, s_tmp);
        }
        printf("%d cmd are found", cnt);
    } else {
        if (!has_boincapp) boincapp();
        bool flag = true;
        for (int i=1; i<=N_in; i++)
            flag &= check_inputfile(infile[i]);
        if (flag) printf("1 file case are found");
            else {
                int cnt = 0;
                flag = true;
                while (flag){
                    cnt++;
                    for (int i=1; i<=N_in; i++)
                        flag &= check_inputfile(infile[i])|check_inputfile(infile[i], cnt);
                }
                printf("%d file case are found", cnt-1);
            }
    }
    rebuild_config();
    return 0;
}

