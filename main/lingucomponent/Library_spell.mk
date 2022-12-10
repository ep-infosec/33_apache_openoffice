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



$(eval $(call gb_Library_Library,spell))

$(eval $(call gb_Library_add_precompiled_header,spell,$(SRCDIR)/lingucomponent/inc/pch/precompiled_spell))

$(eval $(call gb_Library_set_componentfile,spell,lingucomponent/source/spellcheck/spell/spell))

$(eval $(call gb_Library_set_include,spell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/inc \
	-I$(SRCDIR)/lingucomponent/inc/pch \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,spell, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,spell,\
	cppu \
	cppuhelper \
	i18nisolang1 \
	lng \
	sal \
	stl \
	svl \
	tl \
	utl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_static_libs,spell,\
	ulingu \
))

$(call gb_Library_use_externals,spell,\
	hunspell \
	icuuc \
)

ifneq ($(OS),MACOSX)
$(eval $(call gb_Library_add_exception_objects,spell,\
	lingucomponent/source/spellcheck/spell/sreg \
	lingucomponent/source/spellcheck/spell/sspellimp \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_libs,spell,\
	-framework Cocoa \
	-framework Carbon \
	-framework CoreFoundation \
))

$(eval $(call gb_Library_add_cxxflags,spell,\
    -x -objective-c++ -stdlib=libc++ \
))
endif
# vim: set noet sw=4 ts=4:

