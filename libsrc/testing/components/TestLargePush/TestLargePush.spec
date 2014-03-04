# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %define _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix:         %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name:           TestLargePush
Version:        1.0.0
Release:        1%{?dist}
Summary:        Component %{name}

Group:          REDHAWK/Components
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 1.9
Requires:       redhawk >= 1.9

# Interface requirements
BuildRequires:  bulkioInterfaces
Requires:       bulkioInterfaces

%description
Component %{name}


%prep
%setup -q


%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dom/components/TestLargePush/cpp
%configure
make %{?_smp_mflags}
popd
# Implementation python
pushd python
./reconf
%define _bindir %{_prefix}/dom/components/TestLargePush/python
%configure
make %{?_smp_mflags}
popd
# Implementation java
pushd java
./reconf
%define _bindir %{_prefix}/dom/components/TestLargePush/java
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dom/components/TestLargePush/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd
# Implementation python
pushd python
%define _bindir %{_prefix}/dom/components/TestLargePush/python
make install DESTDIR=$RPM_BUILD_ROOT
popd
# Implementation java
pushd java
%define _bindir %{_prefix}/dom/components/TestLargePush/java
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/TestLargePush.scd.xml
%{_prefix}/dom/components/%{name}/TestLargePush.prf.xml
%{_prefix}/dom/components/%{name}/TestLargePush.spd.xml
%{_prefix}/dom/components/%{name}/cpp
%{_prefix}/dom/components/%{name}/python
%{_prefix}/dom/components/%{name}/java

