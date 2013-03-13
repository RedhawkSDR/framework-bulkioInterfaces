#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

# By default, the RPM will install to the standard REDHAWK OSSIE root location (/usr/local/redhawk/core)
# You can override this at install time using --prefix /usr/local/redhawk/core when invoking rpm (preferred)
%define _ossiehome /usr/local/redhawk/core
%define _prefix %{_ossiehome}
Prefix: %{_prefix}

# Point install paths to locations within our target OSSIE root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

# Check for java support
%define java_support_enabled %(test -e $OSSIEHOME/lib/ossie.jar && echo 1 || echo 0)

Name:		bulkioInterfaces
Version:	__VERSION__
Release:	__RELEASE__
Summary:	The bulkio library for REDHAWK

Group:		Applications/Engineering
License:	LGPLv3+
URL:		http://redhawksdr.org/
Source:		%{name}-%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires: redhawk >= 1.8
BuildRequires: redhawk >= 1.8
BuildRequires: autoconf automake libtool
BuildRequires: omniORB
BuildRequires: python omniORBpy omniORBpy-devel
BuildRequires: apache-log4cxx-devel
%if "%{?rhel}" == "6"
BuildRequires: libuuid-devel
%else
BuildRequires: e2fsprogs-devel
%endif


%description
Libraries and interface definitions for bulkio interfaces.


%prep
%setup -q


%build
# Explicitly set JAVA_HOME and add to PATH (for idlj)
export JAVA_HOME=/usr/java/default
export PATH=${JAVA_HOME}/bin:${PATH}
./reconf
%configure
make


%install
rm -rf --preserve-root $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf --preserve-root $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk)
%{_datadir}/idl/ossie/BULKIO
%{_includedir}/ossie/BULKIO
%{_libdir}/libbulkioInterfaces.*
%{_libdir}/pkgconfig/bulkioInterfaces.pc
%{_prefix}/lib/python/bulkio
%if %java_support_enabled
%{_prefix}/lib/BULKIOInterfaces.jar
%{_prefix}/lib/BULKIOInterfaces.src.jar
%{_prefix}/%{_lib}/libbulkiojni.*
%endif
%if "%{?rhel}" == "6"
%{_prefix}/lib/python/bulkioInterfaces-0.0.0-py2.6.egg-info
%endif


%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Tue Mar 12 2013 1.8.3-4
- Update licensing information
- Add URL for website
- Change group to a standard one, per Fedora
- Quiet the file unpack

