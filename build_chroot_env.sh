mkdir /var/sandbox
mkdir /var/sandbox/$1/
mkdir /var/sandbox/$1/proc
mkdir /var/sandbox/$1/lib
mkdir /var/sandbox/$1/lib64
mkdir /var/sandbox/$1/usr
mkdir /var/sandbox/$1/usr/lib
mkdir /var/sandbox/$1/usr/bin

cd /var/sandbox/$1
mount -t proc none ./proc
mount --bind /usr/lib ./usr/lib
mount -o remount,nodev,exec,nosuid ./usr/lib
mount --bind /lib ./lib
mount -o remount,nodev,exec,nosuid ./lib
mount --bind /lib64 ./lib64/
mount -o remount,nodev,exec,nosuid ./lib64/
ln -s /usr/bin/mono ./usr/bin/mono
ln -s /usr/bin/java ./usr/bin/java

mkdir /sys/fs/cgroup
mkdir /sys/fs/cgroup/$1
mount -t tmpfs $1 /sys/fs/cgroup/$1 -o size=32M
mount -t cgroup -o cpuacct,memory $1 /sys/fs/cgroup/$1/



