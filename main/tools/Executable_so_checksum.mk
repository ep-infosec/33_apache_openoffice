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



$(eval $(call gb_Executable_Executable,so_checksum))

$(eval $(call gb_Executable_set_include,so_checksum,\
	$$(INCLUDE) \
	-I$(SRCDIR)/tools/inc/ \
	-I$(SRCDIR)/tools/inc/pch \
	-I$(SRCDIR)/tools/bootstrp/ \
))

$(eval $(call gb_Executable_add_defs,so_checksum,\
	-D_TOOLS_STRINGLIST \
	$(STLPORT_DEBUGFLAG) \
))

$(eval $(call gb_Executable_add_linked_libs,so_checksum,\
	sal \
	tl \
    $(gb_STDLIBS) \
))
# used to link against basegfxlx comphelp4gcc3 i18nisolang1gcc3 ucbhelper4gcc3 uno_cppu uno_cppuhelpergcc3 uno_salhelpergcc3 vos3gcc3 - seems to be superficial

$(eval $(call gb_Executable_add_exception_objects,so_checksum,\
	tools/bootstrp/md5 \
	tools/bootstrp/so_checksum \
))

# vim: set noet sw=4 ts=4:
