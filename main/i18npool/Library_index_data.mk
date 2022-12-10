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



$(eval $(call gb_Library_Library,index_data))

$(eval $(call gb_Library_set_include,index_data,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,index_data, \
#	offapi \
#	udkapi \
#))

$(eval $(call gb_Library_set_versionmap,index_data,$(SRCDIR)/i18npool/source/indexentry/data/index_data.map))

$(eval $(call gb_Library_add_linked_libs,index_data,\
	stl \
	$(gb_STDLIBS) \
))

$(WORKDIR)/CustomTarget/i18npool/source/indexentry/data/%.cxx : $(SRCDIR)/i18npool/source/indexentry/data/%.txt $(OUTDIR)/bin/genindex_data$(gb_Executable_EXT)
	mkdir -p $(dir $@) && \
	$(gb_Augment_Library_Path) $(OUTDIR)/bin/genindex_data$(gb_Executable_EXT) $(call gb_Helper_convert_native,$<) $(call gb_Helper_convert_native,$@) $*

$(eval $(call gb_Library_add_generated_exception_objects,index_data,\
	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/indexentry/data/*.txt),CustomTarget/i18npool/source/indexentry/data/$(notdir $(basename $(txt)))) \
))

# vim: set noet sw=4 ts=4:

