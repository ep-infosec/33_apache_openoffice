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



# replace MODULE by the name of the module you are creating

$(eval $(call gb_Module_Module,MODULE))

# the targets to be inserted are their file names without .mk extension
$(eval $(call gb_Module_add_targets,MODULE,\
))

# add any runtime tests (unit tests) here
# remove if no tests
$(eval $(call gb_Module_add_check_targets,MODULE,\
))

# add any subsequent checks (e.g. complex tests) here
$(eval $(call gb_Module_add_subsequentcheck_targets,MODULE,\
))

# vim: set noet sw=4 ts=4:
