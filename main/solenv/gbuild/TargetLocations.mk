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



# outdir target pattern

gb_ComponentTarget_get_outdir_target = $(OUTDIR)/xml/component/$(1).component
gb_ComponentTarget_get_outdir_inbuild_target = $(OUTDIR)/xml/component/$(1).inbuild.component
gb_Executable_get_target = $(OUTDIR)/bin/$(1)$(gb_Executable_EXT)
gb_PackagePart_get_destinations = $(OUTDIR)/xml $(OUTDIR)/inc $(OUTDIR)/bin $(OUTDIR)/lib $(OUTDIR)/idl $(OUTDIR)/pck
gb_PackagePart_get_target = $(OUTDIR)/$(1)
gb_ResTarget_get_outdir_imagelist_target = $(OUTDIR)/res/img/$(1).ilst
gb_ResTarget_get_outdir_target = $(OUTDIR)/bin/$(1).res
gb_Jar_get_outdir_target = $(OUTDIR)/bin/$(1).jar
gb_Zip_get_outdir_target = $(OUTDIR)/pck/$(1).zip

define gb_Library_get_target
$(patsubst $(1):%,$(gb_Library_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_Library_TARGETS)):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_target
$(patsubst $(1):%,$(gb_StaticLibrary_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_StaticLibrary_TARGETS)):%,$(gb_StaticLibrary_FILENAMES)))
endef


# workdir target patterns

gb_AllLangResTarget_get_target = $(WORKDIR)/AllLangRes/$(1)
gb_Ant_get_target = $(WORKDIR)/Ant/$(1).jar
gb_AsmObject_get_target = $(WORKDIR)/AsmObject/$(1).o
gb_BisonTarget_get_target = $(WORKDIR)/Bison/$(1)/$(2).cxx
gb_CObject_get_target = $(WORKDIR)/CObject/$(1).o
gb_ComponentTarget_get_target = $(WORKDIR)/ComponentTarget/$(1).component
gb_ComponentTarget_get_inbuild_target = $(WORKDIR)/ComponentTarget/$(1).inbuild.component
gb_CustomTarget_get_repo_target = $(WORKDIR)/CustomTarget/$(2)_$(1).done
gb_CustomTarget_get_target = $(WORKDIR)/CustomTarget/$(1).done
gb_CustomTarget_get_workdir = $(WORKDIR)/CustomTarget/$(1)
gb_CxxObject_get_target = $(WORKDIR)/CxxObject/$(1).o
gb_GenCObject_get_target = $(WORKDIR)/GenCObject/$(1).o
gb_GenCxxObject_get_target = $(WORKDIR)/GenCxxObject/$(1).o
gb_Executable_get_external_headers_target = $(WORKDIR)/ExternalHeaders/Executable/$(1)
gb_Executable_get_headers_target = $(WORKDIR)/Headers/Executable/$(1)
gb_FlexTarget_get_target = $(WORKDIR)/Flex/$(1)/$(2).cxx
gb_GoogleTest_get_target = $(WORKDIR)/GoogleTest/$(1).test
gb_Jar_get_target = $(WORKDIR)/Jar/$(1).jar
gb_Jar_get_classsetname = Jar/$(1)
gb_JavaClassSet_get_classdir = $(WORKDIR)/JavaClassSet/$(1)
gb_JavaClassSet_get_repo_target = $(WORKDIR)/JavaClassSet/$(2)/$(1).done
gb_JavaClassSet_get_target = $(WORKDIR)/JavaClassSet/$(1)/done
gb_JunitTest_get_classsetname = JunitTest/$(1)
gb_JunitTest_get_target = $(WORKDIR)/JunitTest/$(1)/done
gb_JunitTest_get_userdir = $(WORKDIR)/JunitTest/$(1)/user
gb_LinkTarget_get_external_headers_target = $(WORKDIR)/ExternalHeaders/$(1)
gb_LinkTarget_get_headers_target = $(WORKDIR)/Headers/$(1)
gb_LinkTarget_get_target = $(WORKDIR)/LinkTarget/$(1)
gb_UnoApiTarget_get_target = $(WORKDIR)/UnoApiTarget/$(1).rdb
gb_UnoApiOutTarget_get_target = $(OUTDIR)/bin/$(1).rdb
gb_UnoApiPartTarget_get_target = $(WORKDIR)/UnoApiPartTarget/$(1)
gb_UnoApiTarget_get_header_target = $(WORKDIR)/UnoApiHeaders/$(1)
gb_UnoPrivateApiTarget_get_target = $(WORKDIR)/UnoPrivateApi/$(1)
gb_UnoPrivateApiExtractTarget_get_target = $(WORKDIR)/UnoPrivateApiExtract/$(1)
gb_Module_get_check_target = $(WORKDIR)/Module/check/$(1)
gb_Module_get_subsequentcheck_target = $(WORKDIR)/Module/subsequentcheck/$(1)
gb_Module_get_target = $(WORKDIR)/Module/$(1)
gb_NoexPrecompiledHeader_get_dep_target = $(WORKDIR)/Dep/NoexPrecompiledHeader/$(gb_NoexPrecompiledHeader_DEBUGDIR)/$(1).hxx.pch.d
gb_NoexPrecompiledHeader_get_target = $(WORKDIR)/NoexPrecompiledHeader/$(gb_NoexPrecompiledHeader_DEBUGDIR)/$(1).hxx.pch
gb_ObjCObject_get_target = $(WORKDIR)/ObjCObject/$(1).o
gb_ObjCxxObject_get_target = $(WORKDIR)/ObjCxxObject/$(1).o
gb_Package_get_preparation_target = $(WORKDIR)/Package/prepared/$(1)
gb_Package_get_target = $(WORKDIR)/Package/$(1)
gb_PrecompiledHeader_get_dep_target = $(WORKDIR)/Dep/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/$(1).hxx.pch.d
gb_PrecompiledHeader_get_target = $(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/$(1).hxx.pch
gb_ResTarget_get_imagelist_target = $(WORKDIR)/ResTarget/$(1).ilst
gb_ResTarget_get_target = $(WORKDIR)/ResTarget/$(1).res
gb_SdiTarget_get_target = $(WORKDIR)/SdiTarget/$(1)
gb_SrsPartMergeTarget_get_target = $(WORKDIR)/SrsPartMergeTarget/$(1)
gb_SrsPartTarget_get_target = $(WORKDIR)/SrsPartTarget/$(1)
gb_SrsTarget_get_target = $(WORKDIR)/SrsTarget/$(1).srs
gb_WinResTarget_get_target = $(WORKDIR)/WinResTarget/$(1)$(gb_WinResTarget_POSTFIX)
gb_Zip_get_target = $(WORKDIR)/Zip/$(1).zip
gb_Zip_get_final_target = $(WORKDIR)/Zip/$(1).done

define gb_Library_get_external_headers_target
$(patsubst $(1):%,$(WORKDIR)/ExternalHeaders/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_Library_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_external_headers_target
$(patsubst $(1):%,$(WORKDIR)/ExternalHeaders/StaticLibrary/%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

define gb_StaticLibrary_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/StaticLibrary/%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

$(eval $(call gb_Helper_make_clean_targets,\
	AllLangResTarget \
	Ant \
	ComponentTarget \
	JavaClassSet \
	Jar \
	JunitTest \
	LinkTarget \
	Module \
	WinResTarget \
	NoexPrecompiledHeader \
	PackagePart \
	PrecompiledHeader \
	ResTarget \
	SdiTarget \
	SrsTarget \
	GoogleTest \
	CustomTarget \
	UnoApiTarget \
	Zip \
))

$(eval $(call gb_Helper_make_outdir_clean_targets,\
	Executable \
	Library \
	Package \
	StaticLibrary \
	UnoApiOutTarget \
))

$(eval $(call gb_Helper_make_dep_targets,\
	CObject \
	CxxObject \
	ObjCObject \
	ObjCxxObject \
	GenCObject \
	GenCxxObject \
	LinkTarget \
	SrsPartTarget \
	SrsTarget \
	UnoApiTarget \
))

# other getters

define gb_Library_get_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_LAYER)))
endef

define gb_Executable_get_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Executable_LAYER)))
endef

define gb_Library_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef


# static members declared here because they are used globally

gb_Library_OUTDIRLOCATION = $(OUTDIR)/lib
gb_Library_DLLDIR = $(WORKDIR)/LinkTarget/Library
gb_StaticLibrary_OUTDIRLOCATION = $(OUTDIR)/lib

# vim: set noet sw=4 ts=4:
