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

- Build
-- Setup initial source tree
```
$ autoscan
$ mv configure.scan configure.in
# Edit configure.in...
# Create Makefile.am...
```
-- Rebuild after file/directory added
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
-- Create LUN image directory
```
# mkdir -p /var/lib/iscsi/luns
```
-- Create LUN image file
create 32GB LUN image file.
```
# dd if=/dev/zero of=/var/lib/iscsi/luns/disk1.img count=0 bs=1 seek=32G
```
- sstoraged configuration
sstoraged's configuration file is "/etc/sstorage/sstoraged.conf"
Configuration details are described in https://github.com/EijiSugiura/sstoraged/blob/master/man/sstoraged.conf.pod.

Minimum configuration file is as follows.
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

## Connect to ISCSI target from ISCSI initiator

