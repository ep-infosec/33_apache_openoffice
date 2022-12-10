# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************
# version and release passed by command-line
Version: %version
Release: %release
Summary: %productname desktop integration
Name: %pkgprefix-freedesktop-menus
#BuildRequires: sed
#BuildRequires: perl
Group: Office
License: ALv2
Provides: openoffice-desktop-integration
Conflicts: %pkgprefix-suse-menus
Conflicts: %pkgprefix-debian-menus
Conflicts: %pkgprefix-redhat-menus
Conflicts: %pkgprefix-mandriva-menus
Obsoletes: openoffice.org3.0-freedesktop-menus
Obsoletes: openoffice.org3.1-freedesktop-menus
Obsoletes: openoffice.org3.2-freedesktop-menus
Obsoletes: openoffice.org3.3-freedesktop-menus
Obsoletes: openoffice.org3.4-freedesktop-menus
BuildArch: noarch
AutoReqProv: no
%define _binary_filedigest_algorithm 1
%define _binary_payload w9.gzdio

%description
%productname desktop integration for desktop-environments that implement 
the menu- and mime-related specifications from http://www.freedesktop.org
Install this package if you're using a distribution not covered by any of 
the other %pkgprefix-<distribution>-menus packages.

%install
rm -rf $RPM_BUILD_ROOT

# hack/workaround to make SuSE's brp-symlink-script happy. It wants the targets of all links
# to be present on the build-system/the buildroot. But the point is that we generate stale
# links intentionally (until we find a better solution) #46226
export NO_BRP_STALE_LINK_ERROR=yes

mkdir -p $RPM_BUILD_ROOT

# FIXME: remove - only purpose is to create packages identical to OOF680 m8
umask 0000

# set parameters for the create_tree script 
export DESTDIR=$RPM_BUILD_ROOT
export KDEMAINDIR=/usr
export GNOMEDIR=/usr
export GNOME_MIME_THEME=hicolor

./create_tree.sh

cd $RPM_BUILD_ROOT

# freedesktop-based desktop-environments don't need/use this.
rm -rf usr/share/application-registry
rm -rf usr/share/applications.flag
rm -rf usr/share/mime-info
rm -rf usr/share/mimelnk
rm -rf usr/share/applnk-redhat
#find usr/share/icons -name '*.png' -exec chmod g+w {} \;

%clean
rm -rf $RPM_BUILD_ROOT 

%triggerin -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-math
# this is run when one of the above packages is already installed and the menu
# package gets installed OR when the menu-package is already installed and one
# of the above listed packages gets installed

# Dut to a bug in rpm it is not possible to check why the script is triggered...
# This is how it should be: 1st arg: number of this package, 2nd arg: number of
# package that triggers - the bug is that rpm reports the same number for both
# (the value of the 2nd one), so just run this always...
# http://rhn.redhat.com/errata/RHBA-2004-098.html
# https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=100509

if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
elif (which update-desktop-database); then
  update-desktop-database -q /usr/share/applications
fi

%triggerun -- %pkgprefix, %pkgprefix-writer, %pkgprefix-calc, %pkgprefix-draw, %pkgprefix-impress, %pkgprefix-math
if [ "$1" = "0" ] ; then  
  # the menu-package gets uninstalled/updated - postun will run the command
  exit 0
fi
if [ "$2" = "0" ] ; then  
  # the triggering package gets removed
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
      /opt/gnome/bin/update-desktop-database -q
  elif (which update-desktop-database); then
    update-desktop-database -q /usr/share/applications
  fi
fi

%post 
# no need to run it when updating, since %postun of the old package is run
# afterwards

if [ "$1" = "1" ] ; then  # first install
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
  elif (which update-desktop-database); then
    update-desktop-database -q /usr/share/applications
  fi
  
  if (which update-mime-database); then
    update-mime-database /usr/share/mime
  fi
fi


#run always
for theme in gnome hicolor locolor; do
    if [ -e /usr/share/icons/$theme/icon-theme.cache ] ; then
        # touch it, just in case we cannot find the binary...
        touch /usr/share/icons/$theme
        if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
            /opt/gnome/bin/gtk-update-icon-cache -q /usr/share/icons/$theme 
        elif (which gtk-update-icon-cache); then
            gtk-update-icon-cache -q /usr/share/icons/$theme
        fi
        # ignore errors (e.g. when there is a cache, but no index.theme)
        true
    fi
done


# update /etc/mime.types
# backing out existing entries to avoid duplicates
sed '
/application\/vnd\.oasis\.opendocument/d
/application\/vnd\.sun/d
/application\/vnd\.stardivision/d
/application\/vnd\.openofficeorg/d
' /etc/mime.types 2>/dev/null >> /etc/mime.types.tmp$$

# now append our stuff to the temporary file
cat >> /etc/mime.types.tmp$$ << END
application/vnd.oasis.opendocument.text	odt
application/vnd.oasis.opendocument.text-template ott
application/vnd.oasis.opendocument.text-web oth
application/vnd.oasis.opendocument.text-master odm
application/vnd.oasis.opendocument.graphics odg
application/vnd.oasis.opendocument.graphics-template otg
application/vnd.oasis.opendocument.presentation odp
application/vnd.oasis.opendocument.presentation-template otp
application/vnd.oasis.opendocument.spreadsheet ods
application/vnd.oasis.opendocument.spreadsheet-template ots
application/vnd.oasis.opendocument.chart odc
application/vnd.oasis.opendocument.formula odf
application/vnd.oasis.opendocument.image odi
application/vnd.sun.xml.writer sxw
application/vnd.sun.xml.writer.template stw
application/vnd.sun.xml.writer.global sxg
application/vnd.stardivision.writer sdw vor
application/vnd.stardivision.writer-global sgl
application/vnd.sun.xml.calc sxc
application/vnd.sun.xml.calc.template stc
application/vnd.stardivision.calc sdc
application/vnd.stardivision.chart sds
application/vnd.sun.xml.impress sxi
application/vnd.sun.xml.impress.template sti
application/vnd.stardivision.impress sdd sdp
application/vnd.sun.xml.draw sxd
application/vnd.sun.xml.draw.template std
application/vnd.stardivision.draw sda
application/vnd.sun.xml.math sxm
application/vnd.sun.xml.base odb
application/vnd.stardivision.math smf
application/vnd.openofficeorg.extension oxt
application/vnd.openxmlformats-officedocument.wordprocessingml.document docx
application/vnd.ms-word.document.macroenabled.12 docm
application/vnd.openxmlformats-officedocument.wordprocessingml.template dotx
application/vnd.ms-word.template.macroenabled.12 dotm
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet xlsx
application/vnd.ms-excel.sheet.macroenabled.12 xlsm
application/vnd.openxmlformats-officedocument.spreadsheetml.template xltx
application/vnd.ms-excel.template.macroenabled.12 xltm
application/vnd.openxmlformats-officedocument.presentationml.presentation pptx
application/vnd.ms-powerpoint.presentation.macroenabled.12 pptm
application/vnd.openxmlformats-officedocument.presentationml.template potx
application/vnd.ms-powerpoint.template.macroenabled.12 potm
END

# and replace the original file
mv -f /etc/mime.types.tmp$$ /etc/mime.types 2>/dev/null

# update /etc/mailcap only at initial install
if [ "$1" = 1 ]
then
  # backing out existing entries to avoid duplicates
  sed '
/^# OpenOffice.org/d
/^application\/vnd\.oasis\.opendocument/d
/^application\/vnd\.openofficeorg/d
/^application\/vnd\.sun/d
/^application\/vnd\.stardivision/d
/^application\/vnd\.ms-word/d
/^application\/vnd\.ms-excel/d
/^application\/vnd\.ms-powerpoint/d
/^application\/x-star/d
/excel/d
/ms[-]*word/d
/powerpoint/d
' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # now append our stuff to the temporary file
  cat >> /etc/mailcap.tmp$$ << END
# Apache OpenOffice
application/vnd.oasis.opendocument.text; %unixfilename -view %s
application/vnd.oasis.opendocument.text-template; %unixfilename -view %s
application/vnd.oasis.opendocument.text-web; %unixfilename -view %s
application/vnd.oasis.opendocument.text-master; %unixfilename -view %s
application/vnd.sun.xml.writer; %unixfilename -view %s
application/vnd.sun.xml.writer.template; %unixfilename -view %s
application/vnd.sun.xml.writer.global; %unixfilename -view %s
application/vnd.stardivision.writer; %unixfilename -view %s
application/vnd.stardivision.writer-global; %unixfilename -view %s
application/x-starwriter; %unixfilename -view %s
application/vnd.oasis.opendocument.formula; %unixfilename -view %s
application/vnd.sun.xml.math; %unixfilename -view %s
application/vnd.stardivision.math; %unixfilename -view %s
application/x-starmath; %unixfilename -view %s
application/msword; %unixfilename -view %s
application/vnd.oasis.opendocument.spreadsheet; %unixfilename -view %s
application/vnd.oasis.opendocument.spreadsheet-template; %unixfilename -view %s
application/vnd.sun.xml.calc; %unixfilename -view %s
application/vnd.sun.xml.calc.template; %unixfilename -view %s
application/vnd.stardivision.calc; %unixfilename -view %s
application/x-starcalc; %unixfilename -view %s
application/vnd.stardivision.chart; %unixfilename -view %s
application/x-starchart; %unixfilename -view %s
application/excel; %unixfilename -view %s
application/msexcel; %unixfilename -view %s
application/vnd.ms-excel; %unixfilename -view %s
application/x-msexcel; %unixfilename -view %s
application/vnd.oasis.opendocument.presentation; %unixfilename -view %s
application/vnd.oasis.opendocument.presentation-template; %unixfilename -view %s
application/vnd.sun.xml.impress; %unixfilename -view %s
application/vnd.sun.xml.impress.template; %unixfilename -view %s
application/vnd.stardivision.impress; %unixfilename -view %s
application/x-starimpress; %unixfilename -view %s
application/powerpoint; %unixfilename -view %s
application/mspowerpoint; %unixfilename -view %s
application/vnd.ms-powerpoint; %unixfilename -view %s
application/x-mspowerpoint; %unixfilename -view %s
application/vnd.oasis.opendocument.graphics; %unixfilename -view %s
application/vnd.oasis.opendocument.graphics-template; %unixfilename -view %s
application/vnd.sun.xml.draw; %unixfilename -view %s
application/vnd.sun.xml.draw.template; %unixfilename -view %s
application/vnd.stardivision.draw; %unixfilename -view %s
application/x-stardraw; %unixfilename -view %s
application/vnd.oasis.opendocument.database; %unixfilename -view %s
application/vnd.sun.xml.base; %unixfilename -view %s
application/vnd.openofficeorg.extension; %unixfilename %s
application/vnd.openxmlformats-officedocument.wordprocessingml.document; %unixfilename -view %s
application/vnd.ms-word.document.macroenabled.12;%unixfilename -view %s
application/vnd.openxmlformats-officedocument.wordprocessingml.template; %unixfilename -view %s
application/vnd.ms-word.template.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet; %unixfilename -view %s
application/vnd.ms-excel.sheet.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.spreadsheetml.template; %unixfilename -view %s
application/vnd.ms-excel.template.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.presentationml.presentation; %unixfilename -view %s
application/vnd.ms-powerpoint.presentation.macroenabled.12; %unixfilename -view %s
application/vnd.openxmlformats-officedocument.presentationml.template; %unixfilename -view %s
application/vnd.ms-powerpoint.template.macroenabled.12; %unixfilename -view %s
END

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q
elif (which update-desktop-database); then
  update-desktop-database -q /usr/share/applications
fi

%preun 
# remove from /etc/mailcap only on de-install
if [ "$1" = 0 ]
then
  # backing all entries pointing to our binary
  sed '/%unixfilename/d' /etc/mailcap 2>/dev/null >> /etc/mailcap.tmp$$

  # and replace the original file
  mv -f /etc/mailcap.tmp$$ /etc/mailcap
fi

%postun 
if [ "$1" = 0 ] ; then # only run when erasing the package - other cases handled by the triggers
  if [ -x /opt/gnome/bin/update-desktop-database ]; then
    /opt/gnome/bin/update-desktop-database -q 
  elif (which update-desktop-database); then
    update-desktop-database -q
  fi
# run always - both when upgrading as well as when erasing the package
  if (which update-mime-database); then
    update-mime-database /usr/share/mime
  fi
fi

#run always
for theme in gnome hicolor locolor; do
  if [ -e /usr/share/icons/$theme/icon-theme.cache ] ; then
   # touch it, just in case we cannot find the binary...
    touch /usr/share/icons/$theme
    if [ -x /opt/gnome/bin/gtk-update-icon-cache ]; then
      /opt/gnome/bin/gtk-update-icon-cache -q /usr/share/icons/$theme
    elif (which gtk-update-icon-cache); then
      gtk-update-icon-cache -q /usr/share/icons/$theme
    fi
    # ignore errors (e.g. when there is a cache, but no index.theme)
    true
  fi
done

%files 
# specify stale symlinks verbatim, not as glob - a change in recent versions of 
# glibc breaks rpm unless rpm is build with internal glob-matching (issue 49374)
# https://bugzilla.redhat.com/beta/show_bug.cgi?id=134362
%defattr(-, root, root)
%attr(0755, root, root) /usr/bin/*
/usr/share/applications/%unixfilename-base.desktop
/usr/share/applications/%unixfilename-calc.desktop
/usr/share/applications/%unixfilename-draw.desktop
/usr/share/applications/%unixfilename-impress.desktop
/usr/share/applications/%unixfilename-math.desktop
/usr/share/applications/%unixfilename-printeradmin.desktop
/usr/share/applications/%unixfilename-writer.desktop
/usr/share/applications/%unixfilename-startcenter.desktop
/usr/share/applications/%unixfilename-javafilter.desktop
/usr/share/icons/gnome/*/apps/*png
/usr/share/icons/gnome/*/mimetypes/*png
/usr/share/icons/hicolor/*/apps/*png
/usr/share/icons/hicolor/*/mimetypes/*png
/usr/share/icons/locolor/*/apps/*png
/usr/share/icons/locolor/*/mimetypes/*png
/usr/share/mime/packages/*
