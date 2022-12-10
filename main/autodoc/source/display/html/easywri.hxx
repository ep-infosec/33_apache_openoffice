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



#ifndef ADC_DISPLAY_HTML_EASYWRI_HXX
#define ADC_DISPLAY_HTML_EASYWRI_HXX


// USED SERVICES
	// BASE CLASSES
	// COMPONENTS
#include <estack.hxx>
	// PARAMETERS
#include <udm/html/htmlitem.hxx>


class EasyWriter
{
  public:
    // LIFECYCLE
                        EasyWriter();
                        ~EasyWriter();

    // OPERATIONS
    /// Pushes csi::xml::Element on stack.
    void                Open_OutputNode(
                            csi::xml::Element & io_rDestination );
    /// Pops front csi::xml::Element from stack.
    void                Finish_OutputNode();

    void                Enter(
                            csi::xml::Element & io_rDestination )
                                                { Open_OutputNode(io_rDestination); }
    void                Leave()                 { Finish_OutputNode(); }

    // ACCESS
    csi::xml::Element & Out();                  // CurOutputNode

  private:
	EStack< csi::xml::Element * >
                        aCurDestination;        // The front element is the currently used.
                                                //   The later ones are the parents.
};

/*
inline csi::xml::Element &
EasyWriter::Out()
    { csv_assert( aCurDestination.size() > 0 );
      return *aCurDestination.top(); }
*/

// IMPLEMENTATION


#endif


