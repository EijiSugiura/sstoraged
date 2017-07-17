%define relnum 1
%{expand: %%define release %(if [ -f _dist.spec ]; then echo -n "%{relnum}." ; cat _dist.spec ; else echo %{relnum} ; fi)}

Summary: Software Storage(iSCSI target) daemon
Name: sstoraged
Version: 0.1.3
Release: %{release}
License: GPL
Group: System Environment/Daemons
URL: https://github.com/EijiSugiura/sstoraged
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
sstoraged is an iSCSI target service.

%prep
%setup -q

%build
autoreconf -fvi
ac_cv_func_malloc_0_nonnull=yes ./configure
make

%install
%makeinstall

install -d ${RPM_BUILD_ROOT}/etc/sstorage
install -m 644 redhat/sstoraged.conf ${RPM_BUILD_ROOT}/etc/sstorage/sstoraged.conf
install -m 755 script/make_sstoraged_conf.pl ${RPM_BUILD_ROOT}/etc/sstorage/make_sstoraged_conf.pl

install -d ${RPM_BUILD_ROOT}/etc/sysconfig
install -m 755 redhat/sstoraged.opt ${RPM_BUILD_ROOT}/etc/sysconfig/sstoraged

install -d ${RPM_BUILD_ROOT}/etc/rc.d/init.d
install -m 755 redhat/sstoraged.init ${RPM_BUILD_ROOT}/etc/rc.d/init.d/sstoraged

install -m 755 -d ${RPM_BUILD_ROOT}/var/log/sstoraged
install -m 700 -d ${RPM_BUILD_ROOT}/var/spool/sstoraged

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%config(missingok) /etc/sstorage/sstoraged.conf
%config /etc/sstorage/make_sstoraged_conf.pl
%config /etc/rc.d/init.d/sstoraged
%config(missingok) /etc/sysconfig/sstoraged
%{_sbindir}/sstoraged
%{_mandir}/man5/sstoraged.5.gz
%{_mandir}/man5/sstoraged.conf.5.gz
%doc man/sstorage.css
%doc man/sstoraged.html
%doc man/sstoraged.conf.html
%attr(0755,root,root)	/var/log/sstoraged
%attr(0700,root,root)	/var/spool/sstoraged

%changelog
* Mon Jul 13 2017 Eiji Sugiura <eiji.sugiura@gmail.com> - 0.1.3
- Update for CentOS 6.9


* Mon Jul 23 2007 Eiji Sugiura <sugiura@isp.co.jp> - 
- Initial build.

