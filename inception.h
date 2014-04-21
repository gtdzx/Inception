#include <iostream>
#include <string>
#include <fstream>
using namespace std;
class Inception {
public:
    struct {
        int box_id; 
        string cmd_line,inf,outf,errf;
        int uid, gid;
        string chroot_dir, working_dir, cgroup_dir;
        int time_limit; //in ms
        int memory_limit; //in MB
        long long output_limit; //in Byte
    } architecture;
    int pid;
    char* cmd[20];
    string logf;
    ofstream logger;
    bool alarmed;
    int time; //in ms
    int memory; //in KB
    int exit_status;
    string verdict;
    string int2string(int x);
    bool is_file(string path);
    bool is_dir(string path);
    void log(string);
    int prepare_cmd();
    int check();
    int build();
    int clean();
    int set_nofile();
    int set_nproc();
    int set_memory_limit();
    int set_output_limit();
    int set_stdin();
    int set_stdout();
    int set_stderr();
    int set_cwd();
    int set_time_limit();
    int join_cgroup();
    int set_alarm();
    int read_time();
    int read_memory();
public:
    int init(   int box_id,
                string cmd_line,
                string inf,
                string outf,
                string errf,
                int uid,
                int gid,
                string chroot_dir,
                string working_dir, 
                string cgroup_dir,
                int time_limit,
                int memory_limit,
                long long outout_limit,
                string logf);
    int exec();
    int waitfor();
    int destroy();
};
