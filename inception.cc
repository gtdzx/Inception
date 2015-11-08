#include "inception.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sstream>
#include <vector>
#include <cstdlib>
string Inception::int2string(int x) {
    string ret = "";
    if(x == 0) return "0";
    while(x) {
        char c = '0' + (x % 10);
        ret = c + ret;
        x /= 10;
    }
    return ret;
}

bool Inception::is_file(string path) {
    struct stat _stat;
    if(0 != stat(path.c_str(), &_stat)) {
        string message = "failed to stat " + path + ". errno = " + this->int2string(errno);
        log(message);
        return false;
    }
    return S_ISREG(_stat.st_mode);
}


bool Inception::is_dir(string path) {
    struct stat _stat;
    if(0 != stat(path.c_str(), &_stat)) {
        string message = "failed to stat " + path + ". errno = " + this->int2string(errno);
        log(message);
        return false;
    }
    return S_ISDIR(_stat.st_mode);
}

void Inception::log(string s) {
    if(this->logger.is_open())
        this->logger << s << endl << flush;
    else
        cerr << s << endl << flush;
}
int Inception::prepare_cmd() {
    basic_istringstream<char> in(this->architecture.cmd_line);
    string temp;
    int index = 0;
    while(in >> temp) {
        this->cmd[index] = new char[temp.size() + 1];
        sprintf(this->cmd[index], "%s", temp.c_str());
        index++;
    }
    this->cmd[index] = 0;
}
int Inception::init(int box_id,
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
                long long output_limit,
                string logf) {
    
    this->architecture.box_id = box_id;
    this->architecture.cmd_line = cmd_line;
    this->prepare_cmd();
    this->architecture.inf = inf;
    this->architecture.outf = outf;
    this->architecture.errf = errf;
    this->architecture.uid = uid;
    this->architecture.gid = gid;
    this->architecture.chroot_dir = chroot_dir;
    if(chroot_dir[chroot_dir.size() - 1] != '/')
        this->architecture.chroot_dir += '/';
    this->architecture.working_dir = working_dir;
    if(working_dir[working_dir.size() - 1] != '/')
        this->architecture.working_dir += '/';
    this->architecture.cgroup_dir = cgroup_dir;
    if(cgroup_dir[cgroup_dir.size() - 1] != '/')
        this->architecture.cgroup_dir += '/';
    this->architecture.time_limit = time_limit;
    this->architecture.memory_limit = memory_limit;
    this->architecture.output_limit = output_limit;
    this->logf = logf;
    if(this->logf != "")
        this->logger.open(logf.c_str());

    return 0;
}

int Inception::check() {
    //root privilege;
    uid_t _uid = getuid();
    gid_t _gid = getgid();
    if(_uid != 0 || _gid != 0) {
        string message = "Inception needs root priviege. Current uid:gid is " + this->int2string(_uid) + ':' + this->int2string(_gid);
        log(message);
        return -1;
    }
    //inf
    if(!this->is_file(this->architecture.inf)) {
        log(this->architecture.inf + " is not a regular file.");
        return -2;
    }
    //chroot_dir & working_dir
    if(!this->is_dir(this->architecture.chroot_dir)) {
        log(this->architecture.chroot_dir + " is not a directory.");
        return -3;
    }
    if(!this->is_dir(this->architecture.working_dir)) {
        log(this->architecture.working_dir + " is not a directory.");
        return -3;
    }
    //if working_dir is a subdir of chroot_dir
    string _chroot_dir = this->architecture.chroot_dir.substr(0, this->architecture.chroot_dir.size() - 1);
    if(this->architecture.working_dir.find(_chroot_dir) != 0) {
        log(this->architecture.working_dir + " is not a subdir of " + this->architecture.chroot_dir);
        return -4;
    }
    //cgroup_dir
    if(!this->is_dir(this->architecture.cgroup_dir)) {
        log(this->architecture.cgroup_dir + " is not a directory.");
        return -5;
    }
    return 0;
}

int Inception::set_nofile() {
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = 50;
    return setrlimit(RLIMIT_NOFILE, &rl);
}
int Inception::set_nproc() {
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = 50;
    return setrlimit(RLIMIT_NPROC, &rl);
}
int Inception::set_memory_limit() {
    //echo $memory_limit > memory.limit_in_bytes
    //echo $memory_limit > memory.soft_limit_in_bytes
    FILE* f;
    string path;
    path = this->architecture.cgroup_dir + "memory.limit_in_bytes";
    long long memory_limit_in_bytes = this->architecture.memory_limit;
    memory_limit_in_bytes *= 1024 * 1024;
    f = fopen(path.c_str(), "w");
    if(f == NULL) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%lld", memory_limit_in_bytes);
    fclose(f);
    path = this->architecture.cgroup_dir + "memory.soft_limit_in_bytes";
    f = fopen(path.c_str(), "w");
    if(f == NULL) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%lld", memory_limit_in_bytes);
    fclose(f);
    return 0;
}
int Inception::set_output_limit() {
    struct rlimit rl;
    rl.rlim_cur = this->architecture.output_limit;
    rl.rlim_max = rl.rlim_cur + 1000; //hard limit exceed will cause a SIGKILL instead of a SIGXFSZ, +1000 to make sure parent will receive a SIGXFSZ provided that the child would not change the sigaction.  
    return setrlimit(RLIMIT_FSIZE, &rl);
}
int Inception::set_stdin() {
    int fd, x;
    if(-1 == (fd = open(this->architecture.inf.c_str(), O_RDONLY, 0))) {
        string message = "failed to open " + this->architecture.inf + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    if(-1 == (x = dup2(fd, 0))) {
        string message = "failed to dup2(" + this->int2string(fd) + ",0). errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    return 0;
}
int Inception::set_stdout() {
    int fd, x;
    if(-1 == (fd = open(this->architecture.outf.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644))) {
        string message = "failed to open " + this->architecture.outf + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    if(-1 == (x = dup2(fd, 1))) {
        string message = "failed to dup2(" + this->int2string(fd) + ",1). errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    return 0;
}
int Inception::set_stderr() {
    int fd, x;
    if(-1 == (fd = open(this->architecture.errf.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644))) {
        string message = "failed to open " + this->architecture.errf + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    if(-1 == (x = dup2(fd, 2))) {
        string message = "failed to dup2(" + this->int2string(fd) + ",2). errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    return 0;
}
int Inception::set_cwd() {
    return chdir(this->architecture.working_dir.c_str());
}
int Inception::set_time_limit() {
    struct rlimit rl;
    rl.rlim_cur = this->architecture.time_limit / 1000 + 1;
    rl.rlim_max = rl.rlim_cur + 1; //hard limit exceed will cause a SIGKILL instead of a SIGXCPU. +1 to make sure the parent will receive a SIGXCPU provided that the child would not change the sigaction.
    return setrlimit(RLIMIT_CPU, &rl);
}
int Inception::join_cgroup() {
    //echo pid > tasks
    FILE* f;
    string path;
    path = this->architecture.cgroup_dir + "tasks";
    if(NULL == (f = fopen(path.c_str(), "w"))) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%d", this->pid);
    fclose(f);
    return 0;
}

int Inception::build() {
    int x;
    //sudo mount -t tmpfs none /sys/fs/cgroup/ -o size=32M
    //mkdir /sys/fs/cgroup/cpumem
    //sudo mount -t cgroup -o cpuacct,memory cpumem /sys/fs/cgroup/cpumem
    //mkdir /sys/fs/cgroup/cpumem/box1
    //this step should be completed externally. so we do not need to do this for each exec()
    /*
    if(0 != (x = mount("none", this->architecture.cgroup_dir.c_str(), "tmpfs", 0, "size=32M"))) {
        message = "";
        log(message);
        return x;
    }*/
    //unshare CLONE_NEWIPC
    if(0 != (x = unshare(CLONE_NEWIPC))) {
        string message = "failed to unshare CLONE_NEWIPC. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //unshare CLONE_NEWNET
    if(0 != (x = unshare(CLONE_NEWNET))) {
        string message = "failed to unshare CLONE_NEWNET. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //unshare CLONE_NEWNS
    if(0 != (x = unshare(CLONE_NEWNS))) {
        string message = "failed to unshare CLONE_NEWNS. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //unshare CLONE_NEWPID ?? no details in manual
    //set nofile
    if(0 != (x = this->set_nofile())) {
        string message = "failed to set nofile. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set nproc
    if(0 != (x = this->set_nproc())) {
        string message = "failed to set npoc. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set memory limit
    if(0 != (x = this->set_memory_limit())) {
        string message = "failed to set memory limit. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set output limit
    if(0 != (x = this->set_output_limit())) {
        string message = "failed to set output limit. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set stdin
    if(0 != (x = this->set_stdin())) {
        string message = "failed to set stdin. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set stdout
    if(0 != (x = this->set_stdout())) {
        string message = "failed to set stdout. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set stderr
    if(0 != (x = this->set_stderr())) {
        string message = "failed to set stderr. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //set cwd
    if(0 != (x = this->set_cwd())) {
        string message = "failed to set cwd " + this->architecture.working_dir + ". errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //join cgroup
    if(0 != (x = this->join_cgroup())) {
        string message = "failed to join cgroup. errno = " + this->int2string(errno);
        log(message);
        return x;
    } 
    //chroot
    if(0 != (x = chroot(this->architecture.chroot_dir.c_str()))) {
        string message = "failed to chroot to " + this->architecture.chroot_dir + ". errno = " + this->int2string(errno);
        log(message);
        return 0;
    }
    //set time limit (both cpu time and real time)
    if(0 != (x = this->set_time_limit())) {
        string message = "failed to set time limit. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //drop root privilege:
    if(0 != (x = setresgid(this->architecture.gid, this->architecture.gid, this->architecture.gid))) {
        string message = "failed to set resgid. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    if(0 != (x = setresuid(this->architecture.uid, this->architecture.uid, this->architecture.uid))) {
        string message = "failed to set resuid. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    log("start to close fds...");
    //close other fds
    for(int i = 3; i < 128; i++)
        close(i);

    return 0;
}

int Inception::exec() {
    int x;
    
    if(0 != (x = this->check()))
        return x;
    
    this->pid = fork();
    if(this->pid == 0) {//child process
        if(0 != (x = this->build()))
            exit(x);
        execv(cmd[0], cmd);
        string message = "failed to set execv cmd. errno = " + this->int2string(errno);
        log(message);
        exit(-1);
    }
    //parent process

    return 0;
}

void sig_alarm(int signo) { //free function, do nothing but intr wait
    return;
}

int Inception::set_alarm() {
    int x;
    struct sigaction newact, oldact;
    newact.sa_handler = sig_alarm;
    sigemptyset(&newact.sa_mask);   //no SA_RESTART, make sure to intr wait
    newact.sa_flags = 0;
    if(-1 == (x = sigaction(SIGALRM, &newact, &oldact))) {
        string message = "failed to sigaction. errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    this->alarmed = false;
    alarm(0);
    alarm(this->architecture.time_limit / 1000 + 1); // +1 seconds 
    string message = "set alarm: " + this->int2string(this->architecture.time_limit / 1000 + 1) + " second(s).";
    log(message);
    return 0;
}

int Inception::read_time() {
    string path = this->architecture.cgroup_dir + "cpuacct.usage";
    FILE* f = fopen(path.c_str(), "r");
    long long nano;
    fscanf(f, "%lld", &nano);
    fclose(f);
    return nano / 1000000;
}

int Inception::read_memory() {
    string path = this->architecture.cgroup_dir + "memory.max_usage_in_bytes";
    FILE* f = fopen(path.c_str(), "r");
    long long memory_in_bytes;
    fscanf(f, "%lld", &memory_in_bytes);
    fclose(f);
    return memory_in_bytes / 1000000;
}

int Inception::waitfor() {
    int status;
    int x;
    
    if(-1 == (x = this->set_alarm())) {
        return -1;
    }
    
    //If a signal handler is invoked while a system call or library function call is blocked, then either:
    //* the call is automatically restarted after the signal handler returns; or
    //* the call fails with the error EINTR.
    //We want waitpid below fails with EINTR when sigalarm raises. So we know the child process timed out.
    //See signal(7) and http://stackoverflow.com/questions/282176/waitpid-equivalent-with-timeout for more information
    int _pid = waitpid(this->pid, &status, 0);
    if(_pid != this->pid) { 
        if(errno == EINTR) { //wait intred by alarm
            alarm(0);
            this->alarmed = true;
            this->time = this->architecture.time_limit;
            this->memory = this->read_memory();
            this->verdict = "TLE";
            kill(this->pid, SIGKILL);
            string message = "alarmed. verdict = TLE";
            log(message);
            return 0;
        }
        string message= "failed to waitpid. return value is " + this->int2string(_pid) + ", errno = " + this->int2string(errno);
        log(message);
        alarm(0);
        return -1;
    }
    alarm(0);
    if(WIFEXITED(status)) {//normal exit
        this->exit_status = WEXITSTATUS(status);
        this->time = this->read_time();
        this->memory = this->read_memory();
        if(this->exit_status == 0) {
            if(this->time > this->architecture.time_limit)
                this->verdict = "TLE";
            else if(this->memory > this->architecture.memory_limit)
                this->verdict = "MLE";
            else 
                this->verdict = "OK";
        }
        else {
            this->verdict = "RE";
            //TODO: is it possible that a java (or mono, php, etc) program calls for 10G mem and jvm exits with oom?
            //TODO: read runtime error message from stderr?
        }
        string message = "normal exited with status = " + this->int2string(this->exit_status) + "\nverdict = " + this->verdict;
        log(message);
        return 0;
    }
    if(WIFSIGNALED(status)) {
        int signo = WTERMSIG(status);
        if(signo == SIGXCPU) {//time limit exceeded
            this->time = this->read_time();
            this->memory= this->read_memory();
            this->verdict = "TLE";
        }
        else if(signo == SIGXFSZ) {//output limit exceeded
            this->time = this->read_time();
            this->memory = this->read_memory();
            this->verdict = "OLE";
        }
        /*
        if(this->alarmed) {//time limit exceeded
            this->time = this->architecture.time_limit;
            this->memory = this->read_memory();
            this->verdict = "TLE";
        }*/
        else {//unknown kill
            this->time = this->read_time();
            this->memory = this->read_memory();
            if(this->time > this->architecture.time_limit)
                this->verdict = "TLE";
            else if(this->memory > this->architecture.memory_limit)
                this->verdict = "MLE";
            else
                this->verdict = "RE";
        }
        string message = "signaled by sig = " + this->int2string(signo) + "\nverdict = " + this->verdict;
        log(message);
        return 0;
    }
    log("unknown wait status: neither EXITED nor SIGNALED.");
    return -1;
}

int Inception::destroy() {
    kill(this->pid, SIGKILL);
    kill(-this->pid, SIGKILL);
    FILE* f;
    string path = this->architecture.cgroup_dir + "tasks";
    if(NULL == (f = fopen(path.c_str(), "r"))) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    vector<int> pids;
    int _pid;
    while(fscanf(f, "%d", &_pid) != EOF) {
        pids.push_back(_pid);
    }
    fclose(f);
    if(pids.size() == 0)
        return 0;
    //stop before kill to avoid fork bomb keeping forking...
    for(vector<int>::iterator i = pids.begin(); i != pids.end(); i++) {
        int _tmp = kill(*i, SIGSTOP);
	if(_tmp != 0) {
        	string message = "failed to stop process. errno = " + this->int2string(errno) + ", kill returns " + this->int2string(_tmp);
        	log(message);
	}
    }	
    for(vector<int>::iterator i = pids.begin(); i != pids.end(); i++) {
        int _tmp = kill(*i, SIGKILL);
	if(_tmp != 0) {
        	string message = "failed to kill process. errno = " + this->int2string(errno) + ", kill returns " + this->int2string(_tmp);
        	log(message);
	}
    }	
    pids.clear();
    if(NULL == (f = fopen(path.c_str(), "r"))) {
        string message = "failed to re- open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    while(fscanf(f, "%d", &_pid) != EOF) {
        pids.push_back(_pid);
	string message = "notkilled pid = " + this->int2string(_pid);
	log(message);
    }
    fclose(f);
    return pids.size();
}

int Inception::clean() {
    //echo 0 > cpuacct.usage
    //echo 0 > memory.max_usage_in_bytes
    FILE* f;
    string path;
    path = this->architecture.cgroup_dir + "cpuacct.usage";
    f = fopen(path.c_str(), "w");
    if(f == NULL) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%d", 0);
    fclose(f);
///////////////////////////
//Echo 0 > memory.max_usage_in_bytes is not working. 
//Page cache still counts.
//See http://blogs.360.cn/360xitong/2013/05/02/mem-cgroup%E7%9B%AE%E5%BD%95%E6%97%A0%E6%B3%95%E6%B8%85%E7%90%86%E9%97%AE%E9%A2%98%E5%88%86%E6%9E%90/
//https://www.kernel.org/doc/Documentation/cgroups/memory.txt
//However, force_empty is too slow. It takes about 500ms. So we only force_empty when 
//memory usage > 0.5 * memory limit.
if(this->memory * 2 > this->architecture.memory_limit) {
    path = this->architecture.cgroup_dir + "memory.force_empty";
    f = fopen(path.c_str(), "w");
    if(f == NULL) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%d", 0);
    fclose(f);
}
    path = this->architecture.cgroup_dir + "memory.max_usage_in_bytes";
    f = fopen(path.c_str(), "w");
    if(f == NULL) {
        string message = "failed to open " + path + ". errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    fprintf(f, "%d", 0);
    fclose(f);

    return 0;
}
