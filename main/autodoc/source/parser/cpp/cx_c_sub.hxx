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



#ifndef ADC_CPP_CX_C_SUB_HXX
#define ADC_CPP_CX_C_SUB_HXX

// USED SERVICES
	// BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
	// COMPONENTS
	// PARAMETERS


namespace cpp {


class Context_Comment : public Cx_Base
{
  public:
						Context_Comment(
							TkpContext &		i_rFollowUpContext )
												: 	Cx_Base(&i_rFollowUpContext) {}
	virtual void		ReadCharChain(
							CharacterSource &	io_rText );
	void	   			SetMode_IsMultiLine(
							bool				i_bTrue )
												{ bCurrentModeIsMultiline = i_bTrue; }
  private:
	bool				bCurrentModeIsMultiline;
};

class Context_ConstString : public Cx_Base
{
  public:
						Context_ConstString(
							TkpContext &		i_rFollowUpContext )
												: 	Cx_Base(&i_rFollowUpContext) {}
	virtual void		ReadCharChain(
							CharacterSource &	io_rText );
};

class Context_ConstChar : public Cx_Base
{
  public:
						Context_ConstChar(
							TkpContext &		i_rFollowUpContext )
												: 	Cx_Base(&i_rFollowUpContext) {}
	virtual void		ReadCharChain(
							CharacterSource &	io_rText );
};

class Context_ConstNumeric : public Cx_Base
{
  public:
						Context_ConstNumeric(
							TkpContext &		i_rFollowUpContext )
												: 	Cx_Base(&i_rFollowUpContext) {}
	virtual void		ReadCharChain(
							CharacterSource &	io_rText );
};

class Context_UnblockMacro : public Cx_Base
{
  public:
						Context_UnblockMacro(
							TkpContext &		i_rFollowUpContext )
												: 	Cx_Base(&i_rFollowUpContext) {}
	virtual void		ReadCharChain(
							CharacterSource &	io_rText );
};



}   // namespace cpp


#endif

