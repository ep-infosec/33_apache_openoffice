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



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#ifndef WW_STATICASSERT_HXX
#define WW_STATICASSERT_HXX

/*
 Lifted direct from:
 Modern C++ Design: Generic Programming and Design Patterns Applied
 Section 2.1
 by Andrei Alexandrescu
*/
namespace ww
{
    template<bool> class compile_time_check
    {
    public:
        compile_time_check(...) {}
    };

    template<> class compile_time_check<false>
    {
    };
}

    /*
    Similar to assert, StaticAssert is only in operation when NDEBUG is not
    defined. It will test its first argument at compile time and on failure
    report the error message of the second argument, which must be a valid c++
    classname. i.e. no spaces, punctuation or reserved keywords.
    */
#ifndef NDEBUG
#   define StaticAssert(test, errormsg)                         \
    do {                                                        \
        struct ERROR_##errormsg {};                             \
        typedef ww::compile_time_check< (test) != 0 > tmplimpl; \
        tmplimpl aTemp = tmplimpl(ERROR_##errormsg());          \
        sizeof(aTemp);                                          \
    } while (0)
#else
#   define StaticAssert(test, errormsg)                         \
    do {} while (0)
#endif

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
