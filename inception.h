#include<iostream>
#include<string>
using namespace std;
class Inception {
private:
    struct {
        int box_id; 
        string cmd_line,inf,outf;
        int uid, gid;
        string chroot_dir, working_dir, cgroup_dir;
        int pid;
        string log;
    } architecture;
    int check();
    int build();
    int clean();
public:
    int init(   int box_id,
                string cmd_line,
                string inf,
                string outf,
                int uid,
                int gid,
                string chroot_dir,
                string working_dir, 
                string cgroup_dir,
                string log);
    int exec();
    int waitfor();
    int destroy();
};
