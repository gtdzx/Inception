#include "inception.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
string Inception::int2string(int x) {
    string ret = "";
    while(x) {
        char c = '0' + (x % 10);
        ret = c + ret;
        x /= 10;
    }
    return ret;
}

bool is_file(string path) {
    struct stat _stat;
    if(0 != stat(path.c_str(), &_stat)) {
        string message = "failed to stat " + path + ". errno = " + this->int2string(errno);
        log(message);
        return false;
    }
    return S_ISREG(_stat.st_mode);
}


bool is_dir(string path) {
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

int Inception::init(int box_id,
                string cmd_line, 
                string inf,
                string outf,
                int uid,
                int gid,
                string chroot_dir,
                string working_dir,
                string cgroup_dir,
                string logf) {
    
    this->architecture.box_id = box_id;
    this->architecture.cmd_line = cmd_line;
    this->architecture.inf = inf;
    this->architecture.outf = outf;
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
    //inf & outf
    if(!this->is_file(this->architecture.inf)) {
        log(this->architecture.inf + " is not a regular file.");
        return -2;
    }
    if(!this->is_file(this->architecture.outf)) {
        log(this->architecture.outf + " is not a regular file.");
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
    //chroot
    if(0 != (x = chroot(chroot_dir.c_str()))) {
        string message = "failed to chroot to " + chroot_dir + ". errno = " + this->int2string(errno);
        log(message);
        return 0;
    }
    //set time limit (both cpu time and real time)
    if(0 != (x = this->set_time_limit())) {
        string message = "failed to set time limit. errno = " + this->int2string(errno);
        log(message);
        return x;
    }
    //join cgroup
    if(0 != (x = this->join_cgroup())) {
        string message = "failed to join cgroup. errno = " + this->int2string(errno);
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

    return 0;
}

int Inception::exec() {
    int x;
    
    if(0 != (x = this->check()))
        return x;
    
    this->pid = fork();
    if(this->pid == 0) {//child process
        if(0 != (x = this->build()))
            return x;
        execv(cmd[0], cmd);
        string message = "failed to set execv cmd. errno = " + this->int2string(errno);
        log(message);
        return -1;
    }
    //parent process

    return 0;
}

int Inception::waitfor() {
    return 0;
}

int Inception::destroy() {
    return 0;
}

int Inception::clean() {
    return 0;
}
