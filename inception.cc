#include "inception.h"

int Inception::init(int box_id,
                string cmd_line, 
                string inf,
                string outf,
                int uid,
                int gid,
                string chroot_dir,
                string working_dir,
                string cgroup_dir,
                string log) {
    
    this->architecture.box_id = box_id;
    this->architecture.cmd_line = cmd_line;
    this->architecture.inf = inf;
    this->architecture.outf = outf;
    this->architecture.uid = uid;
    this->architecture.gid = gid;
    this->architecture.chroot_dir = chroot_dir;
    this->architecture.working_dir = working_dir;
    this->architecture.cgroup_dir = cgroup_dir;
    this->architecture.log = log;
    return 0;
}

int Inception::check() {
    return 0;
}

int Inception::build() {
    return 0;
}

int Inception::exec() {
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
