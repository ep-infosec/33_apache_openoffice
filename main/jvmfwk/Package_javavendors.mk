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



$(eval $(call gb_Package_Package,jvmfwk_javavendors,$(SRCDIR)/jvmfwk/distributions))

ifeq ($(GUI),UNX)
ifeq ($(OS),FREEBSD)
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_freebsd.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_freebsd.xml))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_macosx.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_macosx.xml))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_linux.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_linux.xml))
else
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_unx.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_unx.xml))
endif
else ifeq ($(GUI),WNT)
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_wnt.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_wnt.xml))
else ifeq ($(GUI),OS2)
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,bin/javavendors.xml,OpenOfficeorg/javavendors_os2.xml))
$(eval $(call gb_Package_add_file,jvmfwk_javavendors,lib/javavendors.xml,OpenOfficeorg/javavendors_os2.xml))
else
gb_Output_Error(Unsupported platform.)
endif

