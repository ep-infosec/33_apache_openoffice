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



#ifndef ADC_DISPLAY_HF_NAVI_MAIN_HXX
#define ADC_DISPLAY_HF_NAVI_MAIN_HXX



// USED SERVICES
	// BASE CLASSES
	// COMPONENTS
#include "htmlfactory.hxx"
	// PARAMETERS


class HF_MainItem;


/** @task
    Create a HTML navigation bar with lightly coloured background. 

    @descr
    There are three kinds of items:
    Item with link:             Add_StdItem(),   
    Item without link:          Add_NoneItem(),
    Item that is current page:  Add_SelfItem().
*/
class HF_NaviMainRow : public HtmlMaker
{
  public:
    enum E_Style
    {
        eStd,
        eSelf,
        eNo
    };
                        HF_NaviMainRow(
                            Xml::Element &      o_out );
                        ~HF_NaviMainRow();

    void                Add_StdItem(
                            const char *        i_sText,
                            const char *        i_sLink );
    void                Add_SelfItem(
                            const char *        i_sText );
    void                Add_NoneItem(
                            const char *        i_sText );

    void                Produce_Row();

  private:
    // DATA
    typedef std::vector< DYN HF_MainItem* > ItemList;

    ItemList            aItems;
    Xml::Element *      pRow;
};



// IMPLEMENTATION




#endif


