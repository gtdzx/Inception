#include <iostream>
#include <string>
#include <fstream>
using namespace std;
class Inception {
private:
    struct {
        int box_id; 
        string cmd_line,inf,outf;
        int uid, gid;
        string chroot_dir, working_dir, cgroup_dir;
        int pid;
    } architecture;
    string logf;
    ofstream logger;
    string int2string(int x);
    bool is_file(string path);
    bool is_dir(string path);
    void log(string);
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
                string logf);
    int exec();
    int waitfor();
    int destroy();
};
