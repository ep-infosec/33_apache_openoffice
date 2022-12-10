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



#ifndef ADC_DISPLAY_HFI_INTERFACE_HXX
#define ADC_DISPLAY_HFI_INTERFACE_HXX


// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>
#include <ary/doc/d_oldidldocu.hxx>


class HF_NaviSubRow;
class HF_SubTitleTable;
class HF_IdlBaseNode;

class HF_IdlInterface : public HtmlFactory_Idl
{
  public:
                        HF_IdlInterface(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlInterface();

    void                Produce_byData(
                            const client &      i_ce ) const;
  private:
    // Locals
    DYN HF_NaviSubRow & make_Navibar(
                            const client &      i_ce ) const;

    virtual void        produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      ce ) const;
    void                produce_BaseHierarchy(
                            Xml::Element &      o_screen,
                            const client &      i_ce,
                            const String &      i_sLabel ) const;

    // Locals
    enum E_CurProducedMembers
    {
        mem_none,
        mem_Functions,
        mem_Attributes
    };

    // DATA
    mutable E_CurProducedMembers
                        eCurProducedMembers;
};



// IMPLEMENTATION

extern const String
    C_sCePrefix_Interface;



#endif
