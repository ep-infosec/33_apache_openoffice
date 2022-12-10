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



#ifndef ADC_X_PARSE2_HXX
#define ADC_X_PARSE2_HXX

// USED SERVICES
	// BASE CLASSES
#include <cosv/x.hxx>
	// COMPONENTS
	// PARAMETERS


class X_AutodocParser : public csv::Exception
{
  public:
	// TYPES
	enum E_Type
	{
		x_Any						= 0,
		x_InvalidChar,
		x_UnexpectedToken,
		x_UnexpectedEOF
	};
	// LIFECYCLE
						X_AutodocParser(
							E_Type				i_eType,
							const char *		i_sName = "" )
												:	eType(i_eType), sName(i_sName) {}
	// INQUIRY
	virtual void	   	GetInfo(
							std::ostream &      o_rOutputMedium ) const;

  private:
	E_Type				eType;
	String              sName;

};




#endif
