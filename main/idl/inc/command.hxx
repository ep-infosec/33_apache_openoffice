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



#ifndef _COMMAND_HXX
#define _COMMAND_HXX


#include <tools/list.hxx>
#include <tools/string.hxx>

#ifndef STRING_LIST
#define STRING_LIST
DECLARE_LIST( StringList, String * )
#endif

DECLARE_LIST( ByteStringList, ByteString* )

/******************** class SvCommand ************************************/
class SvCommand
{
public:
    String      aDataBaseFile;
    StringList  aInFileList;
    String      aListFile;
    String      aSlotMapFile;
    String      aSfxItemFile;
    String      aODLFile;
    String      aCallingFile;
    String      aCxxFile;
    String      aHxxFile;
    String      aSrcFile;
    String      aPath;
    String      aCHeaderFile;
    String      aCSourceFile;
    String      aTargetFile;
    ByteString      aSrsLine;
    String      aHelpIdFile;
    String      aCSVFile;
    String      aExportFile;
    String      aDocuFile;
    sal_uInt32      nVerbosity;
    sal_uInt32      nFlags;

                SvCommand( int argc, char ** argv );
                ~SvCommand();
};

void Init();
class SvIdlWorkingBase;
sal_Bool ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand );
void DeInit();

#endif // _COMMAND_HXX

