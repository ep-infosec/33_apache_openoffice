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

#include <precomp.h>
#include <ary/loc/loc_filebase.hxx>


// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace loc
{

FileBase::FileBase( const String  &     i_localName,
					Le_id               i_parentDirectory )
    :   sLocalName(i_localName),
        nParentDirectory(i_parentDirectory)
{
}

const String &
FileBase::inq_LocalName() const
{
    return sLocalName;
}

Le_id
FileBase::inq_ParentDirectory() const
{
    return nParentDirectory;
}



} // namespace loc
} // namespace ary
