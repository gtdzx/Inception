#include "inception.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
using namespace std;
int time_limit, case_time_limit, memory_limit;
string run_cmd, validate, working_path, sandbox_path;
int filecount;
vector<string> infiles;
vector<string> outfiles;
vector<string> ansfiles;
void readin()
{
    cin >> time_limit >> case_time_limit >> memory_limit;
    getline(cin, run_cmd);
    getline(cin, run_cmd);
    cin >> validate >> working_path >> sandbox_path;
    cin >> filecount;
    for(int i = 0; i < filecount; i++) {
        string filename;
        cin >> filename;
        infiles.push_back(filename);
        cin >> filename;
        outfiles.push_back(filename);
        cin >> filename;
        ansfiles.push_back(filename);
    }
}
int main()
{
    readin();
    int time_left = time_limit;
    string tlogfile = working_path + "/tlog.txt";
    ofstream tlog(tlogfile.c_str());
    for(int i = 0; i < filecount; i++) {
        tlog << "start testing #" << i << endl;
        int _uid, _gid;
        srand(time(NULL));
        _uid = _gid = rand() % 10000 + 10000;
        string stderr_file = outfiles[i].substr(0, outfiles[i].size() - 3) + "err";
        string logfile = outfiles[i].substr(0, outfiles[i].size() - 3) + "log";
        string resfile = outfiles[i].substr(0, outfiles[i].size() - 3) + "res";
        string incptfile = outfiles[i].substr(0, outfiles[i].size() - 3) + "incpt";
        struct stat file_stat;
        if(stat(ansfiles[i].c_str(), &file_stat) != 0)
        {
            tlog << "failed to stat " << ansfiles[i] << endl;
            tlog.close();
            return -1;
        }
        int output_limit = max(200000, int(file_stat.st_size) * 3);
        int cur_time_limit = min(time_left, case_time_limit);
        Inception* incpt = new Inception();
        int x = 0;
        x = incpt->init(0, run_cmd, infiles[i], outfiles[i], stderr_file, _uid, _gid, sandbox_path,
                sandbox_path, "/sys/fs/cgroup/cpu/sandbox/box0", "/sys/fs/cgroup/memory/sandbox/box0", cur_time_limit, memory_limit, output_limit, incptfile);
        tlog << "init: " << x << endl << flush;
	if(i == 0) {
		incpt->memory = incpt->architecture.memory_limit - 1; //trigger force empty
        	x = incpt->clean();
        	tlog << "first clean: " << x << endl;
	}
        x = incpt->exec();
        tlog << "exec: " << x << endl;
        x = incpt->waitfor();
        tlog << "waitfor: " << x << endl;
	x = incpt->clean();
	tlog << "clean: " << x << endl;
        x = incpt->destroy();
        tlog << "destroy: " << x << endl;
        //x = incpt->clean();
        //tlog << "clean: " << x << endl;

        ofstream log(logfile.c_str());
        log << incpt->verdict << endl;
        log << incpt->time << endl;
        log << incpt->memory << endl;
        log << infiles[i] << endl;
        log.close();
        incpt->logger.close();
        int pid;
        if(incpt->verdict == "OK") {
            tlog << "NORMAL EXITED" << endl;
            if((pid = fork()) < 0) {
                tlog << "Failed to fork validate" << endl;
                tlog.close();
                return -1;
            }
            else {
                if(pid == 0) {//child
                    tlog << "start validating:" << endl;
                    string cmd = validate.substr(validate.find_last_of("\\/")+1, validate.size());
                    if(execlp(validate.c_str(), cmd.c_str(), infiles[i].c_str(), 
                                outfiles[i].c_str(), ansfiles[i].c_str(), logfile.c_str(), resfile.c_str(), (char *) 0) < 0) {
                        tlog << "Failed to exec validate" << endl;
                        tlog.close();
                        return -1;
                    }
                }
                else {
                    int status;
                    waitpid(pid, &status, 0);
                    if(!WIFEXITED(status)) {
                        int sigstop = WSTOPSIG(status);
                        int sig = WTERMSIG(status);
                        tlog << "VALIDATE ERROR" << sig << ' ' << sigstop << endl;
                        tlog.close();
                        return -1;
                    }
                }
            }
            tlog << "VALIDATE FINISHED" << endl;
        }
    }
    tlog.close();
    return 0;
    /*
    string binary = "/home/gtdzx/tmp/fib";
    int x = symlink(binary.c_str(), "/var/sandbox/box0/fib");
    cout << "symlink: " << x << endl;
    int _uid, _gid;
    srand(time(NULL));
    _uid = _gid = rand() % 10000 + 10000;
    Inception* incpt = new Inception();
    x = incpt->init(0, "fib", "/home/gtdzx/tmp/fib.in", "/home/gtdzx/tmp/fib.out", "/home/gtdzx/tmp/fib.err", _uid, _gid, 
            "/var/sandbox/box0", "/var/sandbox/box0", "/sys/fs/cgroup/sandbox/box0",
            10000, 256, 20000, "/home/gtdzx/tmp/box0.log");
    cout << "init: " << x << endl;
    x = incpt->exec();
    cout << "exec: " << x << endl;
    cout << "pid: " << incpt->pid << endl;
    x = incpt->waitfor();
    cout << "waitfor: " << x << endl;
    x = incpt->destroy();
    cout << "destroy: " << x << endl;
    return 0;
    */
}
