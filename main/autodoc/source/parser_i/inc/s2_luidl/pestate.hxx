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



#ifndef ADC_PESTATE_HXX
#define ADC_PESTATE_HXX



// USED SERVICES
	// BASE CLASSES
#include<s2_luidl/tokintpr.hxx>
#include<s2_luidl/tokproct.hxx>
	// COMPONENTS
	// PARAMETERS

namespace csi
{
namespace uidl
{


class TokIdentifier;
class TokBuiltInType;
class TokPunctuation;
class Tok_Documentation;

class ParseEnvState : public   	TokenInterpreter,
					  virtual protected TokenProcessing_Types
{
  public:
	virtual void		Process_Identifier(
							const TokIdentifier &
												i_rToken );
	virtual void		Process_NameSeparator(); 
	virtual void		Process_Punctuation(
							const TokPunctuation &
												i_rToken );
	virtual void		Process_BuiltInType(
							const TokBuiltInType &
												i_rToken );
	virtual void		Process_TypeModifier(
							const TokTypeModifier &
												i_rToken );
	virtual void		Process_MetaType(
							const TokMetaType &	i_rToken );
	virtual void		Process_Stereotype(
							const TokStereotype &
												i_rToken );
	virtual void		Process_ParameterHandling(
							const TokParameterHandling &
												i_rToken );
	virtual void		Process_Raises();
	virtual void		Process_Needs();
	virtual void		Process_Observes();
	virtual void		Process_Assignment(
							const TokAssignment &
												i_rToken );
	virtual void		Process_EOL();

	virtual void		On_SubPE_Left();

	virtual void		Process_Default();

  protected:
						ParseEnvState() 		:	bDefaultIsError(true) {}
	void				SetDefault2Ignore()		{ bDefaultIsError = false; }

  private:
	virtual UnoIDL_PE &	MyPE() = 0;
    bool				bDefaultIsError;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

