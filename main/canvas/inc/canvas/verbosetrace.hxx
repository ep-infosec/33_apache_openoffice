/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef INCLUDED_CANVAS_VERBOSETRACE_HXX
#define INCLUDED_CANVAS_VERBOSETRACE_HXX

#if defined(VERBOSE) && defined(DBG_UTIL)
/// Wrap OSL_TRACE with a verbosity switch
#define VERBOSE_TRACE     OSL_TRACE
#else
#define VERBOSE_TRACE	  1 ? ((void)0) : OSL_TRACE
#endif

#endif /* INCLUDED_CANVAS_VERBOSETRACE_HXX */
