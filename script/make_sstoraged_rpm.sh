#!/bin/bash

REDHAT=`awk '{print $1;}' /etc/redhat-release`
SUB=`awk '{print $2;}' /etc/redhat-release`
DIST="unknown"
if [ $REDHAT == "Red" ]
    then
    DIST="el"`awk '{print $7;}' /etc/redhat-release`
elif [ $REDHAT == "Fedora" ] && [ $SUB == "Core" ]
    then
    DIST="fc"`awk '{print $4;}' /etc/redhat-release`
elif [ $REDHAT == "Fedora" ] && [ $SUB == "release" ]
    then
    DIST="fc"`awk '{print $3;}' /etc/redhat-release`
elif [ $REDHAT == "CentOS" ]
    then
    DIST="cs"`awk '{print $3;}' /etc/redhat-release`
else
    echo "Unknown platform"
    exit 1
fi

if [ ! -d tmp ]
    then
    mkdir -p tmp
fi

echo "cd tmp"
cd tmp

echo "Make sstoraged RPM for $DIST"
echo "$DIST" > _dist.spec

rm -rf ./sstoraged
svn export http://storage.isp.jp/svn/sstoraged/trunk ./sstoraged
VERSION=`grep AM_INIT_AUTOMAKE sstoraged/configure.in | awk -F ", " '{print $2;}'`
echo "VERSION=$VERSION"
rm -rf sstoraged-$VERSION
mv sstoraged sstoraged-$VERSION
tar zcf sstoraged-${VERSION}.tar.gz ./sstoraged-$VERSION

rpmbuild -tb ./sstoraged-${VERSION}.tar.gz
rm _dist.spec
rpmbuild -ts ./sstoraged-${VERSION}.tar.gz
