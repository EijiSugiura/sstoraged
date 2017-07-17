# Software Storage

sstoraged is an iSCSI target built on userland, without kernel drivers.
Still it is experimental and testing only for CentOS6.

## Prerequisite

sstoraged depends on log4cxx. https://logging.apache.org/log4cxx/latest_stable/
Please download log4cxx RPM package from https://github.com/EijiSugiura/sstoraged/tree/master/redhat, and install it.
Or download SRPM, build and install.

## Build and install with RPM package

Download release package from https://github.com/EijiSugiura/sstoraged/releases
```
$ rpmbuild -ta sstoraged-0.X.X.tar.gz
# rpm -Uvh rpmbuild/RPMS/x86_64/sstpraged-0.X.X-1.el6.x86_64.rpm
```

## QUICKLY build and install from source 

```
$ ./configure
$ make
$ make install
```

## Build step by step

- Build from source case 1
    - Setup initial source tree
```
$ autoscan
$ mv configure.scan configure.in
# Edit configure.in...
# Create Makefile.am...
```
- Build from source case 2
    - Rebuild after file/directory added
```
# Edit Makefile.am...
$ autoreconf -fiv
$ ./configure
$ make
```
- Install
```
$ make install
```

## Configuration

- Create LUN image
    - Create LUN image directory ` mkdir -p /var/lib/iscsi/luns`
    - Create LUN image file : create 32GB LUN image file. `# dd if=/dev/zero of=/var/lib/iscsi/luns/disk1.img count=0 bs=1 seek=32G`
- sstoraged configuration : sstoraged's configuration file is "/etc/sstorage/sstoraged.conf"
Configuration details are described in https://github.com/EijiSugiura/sstoraged/blob/master/man/sstoraged.conf.pod.
- Minimum configuration file is as follows.
```
// sstoraged.conf - sstoraged configuration file
{
   "GlobalTargetName" : "iqn.2006-12.com.example:sStorage",
   "LocalTargetName" : "iqn.2006-12.com.example:sStorage0",

   //   LogFile
   //     File pathname, to preserve log messages.
   //     Length MIN. 1 - to - MAX. PATH_MAX
   // "LogFile" : "/var/log/sstoraged/system.log",

   //   LogLevel
   //     Maximum log level, for logging.
   // "LogLevel" : "info",


   "Target" : [
      {
         "TargetName" : "iqn.2006-12.com.example:sStorage0",
         "TargetAlias" : "sStorage0",
         "TargetAddress" : "[0.0.0.0]:3260"
      }
   ],

   "Initiator" : [
      {
         "InitiatorName" : "iqn.2006-12.com.example:iscsi01",
         "InitiatorAlias" : "iscsi01",
         "LUN" : ["LUN1"]
      }
   ],
   "Volume" : [
      {
         "VolumeName" : "LUN1",
         "Main" : [
            {
               "Host" : "iqn.2006-12.com.example:sStorage0",
               "Path" : "/var/lib/iscsi/luns/disk1.img",
               "Start" : 0,
		// Count = LUN_image_size / Block_size = 32 * 1024 * 1024 * 1024 / 512
               "Count" : 67108864
            }
         ]
      }
   ]
}

```

## Start sstoraged iSCSI target

```
[target] ~ # /etc/init.d/sstoraged start
```

## Connect to ISCSI target from ISCSI initiator

Example environment is as follows.
```
   CentOS6                            CentOS6
 [ iSCSI target / sstoraged ] -- 1GbE --- [ iSCSI initiator / iscsi-initiator-utils ]
              192.168.0.254                 192.168.0.1

```

- iSCSI target discovery
```
[initiator] ~ # iscsiadm -m discovery -t sendtargets -p 192.168.0.254
192.168.0.254:3260,1 iqn.2006-12.come.example:sStorage
```
- Login to target
```
[initiator] ~ # iscsiadm -m node -p 192.168.0.254 --login
Logging in to [iface: default, target: iqn.2006-12.com.example:sStorage, portal: 192.168.0.254,3260] (multiple)
Login to [iface: default, target: iqn.2006-com.example.isp:sStorage, portal: 192.168.0.254,3260] successful.
```
- Check the session is defined
```
[initiator] ~ # iscsiadm -m session                                  
tcp: [1] 192.168.0.254:3260,1 iqn.2006-12.com.example:sStorage
```
- Check the partitions, in this case detected as "sdb"
```
[initiator] ~ # cat /proc/partitions
major minor  #blocks  name

   8        0     996912 sda
   8        1      40162 sda1
   8        2     265072 sda2
 254        0     265072 dm-0
 254        1     690795 dm-1
   8       16   33554432 sdb
```
- Check the /var/log/meesages, sStorage is detected.
```
2017-07-16 18:07:01 initiator scsi4 : iSCSI Initiator over TCP/IP
2017-07-16 18:07:01 initiator scsi 4:0:0:0: Direct-Access     PRIV      sStorage         0.1  PQ: 0 ANSI: 6
2017-07-16 18:07:01 initiator sd 4:0:0:0: Attached scsi generic sg1 type 0
2017-07-16 18:07:01 initiator sd 4:0:0:0: [sdb] 67108864 512-byte logical blocks: (34.3 GB/32.0 GiB)
2017-07-16 18:07:01 initiator sd 4:0:0:0: [sdb] Write Protect is off
2017-07-16 18:07:01 initiator sd 4:0:0:0: [sdb] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
```
- Create partitions, as usual
```
[initiator] ~ # fdisk /dev/sdb
```
- Create file system, in this example format as ext4
```
[initiator] ~ # mkfs -t ext4 /dev/sdb1
```
- mount it
```
[initiator] ~ # mount /dev/sdb1 /mnt
```
- Check mount points
```
[initiator] ~ # mount
...
/dev/sdb1 on /var/lib/mysql type ext4 (rw)
```


