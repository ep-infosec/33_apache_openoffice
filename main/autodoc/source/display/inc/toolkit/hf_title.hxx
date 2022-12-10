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



#ifndef ADC_DISPLAY_HF_TITLE_HXX
#define ADC_DISPLAY_HF_TITLE_HXX



// USED SERVICES
	// BASE CLASSES
#include <toolkit/htmlfactory.hxx>
	// COMPONENTS
	// PARAMETERS


class HF_TitleTable : public HtmlMaker
{
  public:
                        HF_TitleTable(
                            Xml::Element &      o_rOut );
    virtual             ~HF_TitleTable();

    void                Produce_Title(
                            const char *        i_title );
    void                Produce_Title(
                            const char *        i_annotations,
//                            const char *        i_label,
                            const char *        i_title );

    /// @return a Html::TableCell reference.
    Xml::Element &      Add_Row();
};


class HF_SubTitleTable : public HtmlMaker
{
  public:
    enum E_SubLevel
    {
        sublevel_1,     /// Big title.
        sublevel_2,     /// Small title.
        sublevel_3      /// No title.
    };

    /** @param i_nColumns [1 .. n]
        @param i_nSubTitleLevel [1 .. 2]
        1 is a bit bigger than 2.
    */

                        HF_SubTitleTable(
                            Xml::Element &      o_rOut,
                            const String &      i_label,
                            const String &      i_title,
                            int                 i_nColumns,
                            E_SubLevel          i_eSubTitleLevel = sublevel_1 );
    virtual             ~HF_SubTitleTable();

    /// @return an Html::TableRow reference.
    Xml::Element &      Add_Row();
};


// IMPLEMENTATION



#endif
