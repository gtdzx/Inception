if [ $# -ne 1 ]; then 
    echo "arg number is not 1"
    exit 1
fi
mkdir /var/sandbox
mkdir /var/sandbox/$1/
mkdir /var/sandbox/$1/proc
mkdir /var/sandbox/$1/lib
mkdir /var/sandbox/$1/lib64
mkdir /var/sandbox/$1/usr
mkdir /var/sandbox/$1/usr/lib
mkdir /var/sandbox/$1/usr/bin
mkdir /var/sandbox/$1/dev

cd /var/sandbox/$1
mount -t proc none ./proc
mount --bind /usr/lib ./usr/lib
mount -o remount,nodev,exec,nosuid ./usr/lib
mount --bind /lib ./lib
mount -o remount,nodev,exec,nosuid ./lib
mount --bind /lib64 ./lib64/
mount -o remount,nodev,exec,nosuid ./lib64/
cp /usr/bin/mono ./usr/bin/mono
cp /usr/lib/jvm/java-7-openjdk-amd64/jre/bin/java ./usr/bin/java
cp /usr/bin/python2.7 ./usr/bin/python2.7
cp /usr/bin/python3.4 ./usr/bin/python3.4
cp /usr/bin/pylint ./usr/bin/pylint
mknod -m 666 ./dev/null c 1 3


mkdir /sys/fs/cgroup/cpu/sandbox
mkdir /sys/fs/cgroup/cpu/sandbox/$1
mkdir /sys/fs/cgroup/memory/sandbox
mkdir /sys/fs/cgroup/memory/sandbox/$1




