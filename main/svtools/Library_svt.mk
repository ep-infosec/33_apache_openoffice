#**************************************************************
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
#**************************************************************



$(eval $(call gb_Library_Library,svt))

$(eval $(call gb_Library_add_package_headers,svt,svtools_inc))

$(eval $(call gb_Library_add_precompiled_header,svt,$(SRCDIR)/svtools/inc/pch/precompiled_svtools))

$(eval $(call gb_Library_set_componentfile,svt,svtools/util/svt))

$(eval $(call gb_Library_add_api,svt,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,svt,\
	-I$(OUTDIR)/inc/external/jpeg \
	$$(INCLUDE) \
	-I$(WORKDIR)/inc/svtools \
	-I$(WORKDIR)/inc/ \
	-I$(OUTDIR)/inc/ \
	-I$(SRCDIR)/svtools/inc \
	-I$(SRCDIR)/svtools/inc/svtools \
	-I$(SRCDIR)/svtools/source/inc \
	-I$(SRCDIR)/svtools/inc/pch \
))

$(eval $(call gb_Library_add_defs,svt,\
	-DSVT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,svt,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	i18nutil \
	jvmfwk \
	sal \
	sot \
	stl \
	svl \
	ootk \
	tl \
	ucbhelper \
	utl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_externals,svt,\
        icuuc \
        jpeg \
)

$(eval $(call gb_Library_add_exception_objects,svt,\
	svtools/source/brwbox/brwbox1 \
	svtools/source/brwbox/brwbox2 \
	svtools/source/brwbox/brwbox3 \
	svtools/source/brwbox/brwhead \
	svtools/source/brwbox/datwin \
	svtools/source/brwbox/ebbcontrols \
	svtools/source/brwbox/editbrowsebox \
	svtools/source/brwbox/editbrowsebox2 \
	svtools/source/config/accessibilityoptions \
	svtools/source/config/apearcfg \
	svtools/source/config/colorcfg \
	svtools/source/config/extcolorcfg \
	svtools/source/config/fontsubstconfig \
	svtools/source/config/helpopt \
	svtools/source/config/htmlcfg \
	svtools/source/config/itemholder2 \
	svtools/source/config/menuoptions \
	svtools/source/config/miscopt \
	svtools/source/config/optionsdrawinglayer \
	svtools/source/config/printoptions \
	svtools/source/contnr/contentenumeration \
	svtools/source/contnr/fileview \
	svtools/source/contnr/imivctl1 \
	svtools/source/contnr/imivctl2 \
	svtools/source/contnr/ivctrl \
	svtools/source/contnr/svicnvw \
	svtools/source/contnr/svimpbox \
	svtools/source/contnr/svimpicn \
	svtools/source/contnr/svlbitm \
	svtools/source/contnr/svlbox \
	svtools/source/contnr/svtabbx \
	svtools/source/contnr/svtreebx \
	svtools/source/contnr/templwin \
	svtools/source/contnr/tooltiplbox \
	svtools/source/contnr/treelist \
	svtools/source/control/accessibleruler \
	svtools/source/control/asynclink \
	svtools/source/control/calendar \
	svtools/source/control/collatorres \
	svtools/source/control/ctrlbox \
	svtools/source/control/ctrltool \
	svtools/source/control/filectrl \
	svtools/source/control/filectrl2 \
	svtools/source/control/fileurlbox \
	svtools/source/control/fixedhyper \
	svtools/source/control/fmtfield \
	svtools/source/control/headbar \
	svtools/source/control/hyperlabel \
	svtools/source/control/indexentryres \
	svtools/source/control/inettbc \
	svtools/source/control/prgsbar \
	svtools/source/control/roadmap \
	svtools/source/control/ruler \
	svtools/source/control/scriptedtext \
	svtools/source/control/scrwin \
	svtools/source/control/stdctrl \
	svtools/source/control/stdmenu \
	svtools/source/control/tabbar \
	svtools/source/control/taskbar \
	svtools/source/control/taskbox \
	svtools/source/control/taskmisc \
	svtools/source/control/taskstat \
	svtools/source/control/toolbarmenu \
	svtools/source/control/toolbarmenuacc \
	svtools/source/control/urlcontrol \
	svtools/source/control/valueacc \
	svtools/source/control/valueset \
	svtools/source/control/vclxaccessibleheaderbar \
	svtools/source/control/vclxaccessibleheaderbaritem \
	svtools/source/dialogs/addresstemplate \
	svtools/source/dialogs/colrdlg \
	svtools/source/dialogs/filedlg \
	svtools/source/dialogs/filedlg2 \
	svtools/source/dialogs/insdlg \
	svtools/source/dialogs/mcvmath \
	svtools/source/dialogs/prnsetup \
	svtools/source/dialogs/property \
	svtools/source/dialogs/roadmapwizard \
	svtools/source/dialogs/wizardmachine \
	svtools/source/dialogs/wizdlg \
	svtools/source/edit/editsyntaxhighlighter \
	svtools/source/edit/svmedit \
	svtools/source/edit/svmedit2 \
	svtools/source/edit/sychconv \
	svtools/source/edit/syntaxhighlight \
	svtools/source/edit/textdata \
	svtools/source/edit/textdoc \
	svtools/source/edit/texteng \
	svtools/source/edit/textundo \
	svtools/source/edit/textview \
	svtools/source/edit/textwindowpeer \
	svtools/source/edit/txtattr \
	svtools/source/edit/xtextedt \
	svtools/source/filter/FilterConfigCache \
	svtools/source/filter/FilterConfigItem \
	svtools/source/filter/SvFilterOptionsDialog \
	svtools/source/filter/exportdialog \
	svtools/source/filter/filter \
	svtools/source/filter/filter2 \
	svtools/source/filter/sgfbram \
	svtools/source/filter/sgvmain \
	svtools/source/filter/sgvspln \
	svtools/source/filter/sgvtext \
	svtools/source/filter/igif/decode \
	svtools/source/filter/igif/gifread \
	svtools/source/filter/ixbm/xbmread \
	svtools/source/filter/ixpm/xpmread \
	svtools/source/filter/jpeg/jpeg \
	svtools/source/filter/wmf/emfwr \
	svtools/source/filter/wmf/enhwmf \
	svtools/source/filter/wmf/winmtf \
	svtools/source/filter/wmf/winwmf \
	svtools/source/filter/wmf/wmf \
	svtools/source/filter/wmf/wmfwr \
	svtools/source/graphic/descriptor \
	svtools/source/graphic/graphic \
	svtools/source/graphic/graphicunofactory \
	svtools/source/graphic/grfattr \
	svtools/source/graphic/grfcache \
	svtools/source/graphic/grfmgr \
	svtools/source/graphic/grfmgr2 \
	svtools/source/graphic/provider \
	svtools/source/graphic/renderer \
	svtools/source/graphic/transformer \
	svtools/source/java/javacontext \
	svtools/source/java/javainteractionhandler \
	svtools/source/misc/acceleratorexecute \
	svtools/source/misc/cliplistener \
	svtools/source/misc/dialogclosedlistener \
	svtools/source/misc/dialogcontrolling \
	svtools/source/misc/ehdl \
	svtools/source/misc/embedhlp \
	svtools/source/misc/embedtransfer \
	svtools/source/misc/helpagentwindow \
	svtools/source/misc/imagemgr \
	svtools/source/misc/imageresourceaccess \
	svtools/source/misc/imap \
	svtools/source/misc/imap2 \
	svtools/source/misc/imap3 \
	svtools/source/misc/itemdel \
	svtools/source/misc/langtab \
	svtools/source/misc/stringtransfer \
	svtools/source/misc/svtaccessiblefactory \
	svtools/source/misc/svtdata \
	svtools/source/misc/templatefoldercache \
	svtools/source/misc/transfer \
	svtools/source/misc/transfer2 \
	svtools/source/misc/unitconv \
	svtools/source/misc/wallitem \
	svtools/source/misc/xwindowitem \
	svtools/source/plugapp/ttprops \
	svtools/source/svhtml/htmlkywd \
	svtools/source/svhtml/htmlout \
	svtools/source/svhtml/htmlsupp \
	svtools/source/svhtml/parhtml \
	svtools/source/svrtf/parrtf \
	svtools/source/svrtf/rtfkeywd \
	svtools/source/svrtf/rtfout \
	svtools/source/svrtf/svparser \
	svtools/source/table/defaultinputhandler \
	svtools/source/table/gridtablerenderer \
	svtools/source/table/tablecontrol \
	svtools/source/table/tablecontrol_impl \
	svtools/source/table/tabledatawindow \
	svtools/source/table/mousefunction \
	svtools/source/table/cellvalueconversion \
	svtools/source/table/tablegeometry \
	svtools/source/toolpanel/drawerlayouter \
	svtools/source/toolpanel/dummypanel \
	svtools/source/toolpanel/paneldecklisteners \
	svtools/source/toolpanel/paneltabbar \
	svtools/source/toolpanel/paneltabbarpeer \
	svtools/source/toolpanel/refbase \
	svtools/source/toolpanel/tabbargeometry \
	svtools/source/toolpanel/tablayouter \
	svtools/source/toolpanel/toolpanel \
	svtools/source/toolpanel/toolpanelcollection \
	svtools/source/toolpanel/toolpaneldeck \
	svtools/source/toolpanel/toolpaneldeckpeer \
	svtools/source/toolpanel/toolpaneldrawer \
	svtools/source/toolpanel/toolpaneldrawerpeer \
	svtools/source/uno/addrtempuno \
	svtools/source/uno/contextmenuhelper \
	svtools/source/uno/framestatuslistener \
	svtools/source/uno/generictoolboxcontroller \
	svtools/source/uno/genericunodialog \
	svtools/source/uno/miscservices \
	svtools/source/uno/popupmenucontrollerbase \
	svtools/source/uno/popupwindowcontroller \
	svtools/source/uno/statusbarcontroller \
	svtools/source/uno/svtxgridcontrol \
	svtools/source/uno/toolboxcontroller \
	svtools/source/uno/treecontrolpeer \
	svtools/source/uno/unocontroltablemodel \
	svtools/source/uno/unogridcolumnfacade \
	svtools/source/uno/unoevent \
	svtools/source/uno/unoiface \
	svtools/source/uno/unoimap \
	svtools/source/uno/wizard/unowizard \
	svtools/source/uno/wizard/wizardpagecontroller \
	svtools/source/uno/wizard/wizardshell \
	svtools/source/urlobj/inetimg \
))

$(eval $(call gb_Library_add_cobjects,svt,\
	svtools/source/filter/jpeg/jpegc \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,svt,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	user32 \
	uuid \
))
endif
# vim: set noet sw=4 ts=4:
