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
#include <tokens/stmstar2.hxx>


// NOT FULLY DECLARED SERVICES
#include <x_parse2.hxx>



StmArrayStatu2::StmArrayStatu2( intt			i_nStatusSize,
								const INT16 * 	in_aArrayModel,
								uintt			i_nTokenId,
							    bool			in_bIsDefault )
	:	dpBranches(new StmStatu2::Branch[i_nStatusSize]),
		nNrOfBranches(i_nStatusSize),
		nTokenId(UINT16(i_nTokenId)),
		bIsADefault(in_bIsDefault)
{
	// KORR_FUTURE: Interface of StmArrayStatu2() has to be changed.
	csv_assert(i_nTokenId < 64536);

	if (in_aArrayModel != 0)
	{
		intt count = 0;
		for (const INT16 * get = in_aArrayModel; count < nNrOfBranches; count++, get++)
			dpBranches[count] = *get;
	}
	else //
	{
		memset(dpBranches, 0, nNrOfBranches);
	}  // endif
}

StmArrayStatu2::~StmArrayStatu2()
{
	delete [] dpBranches;
}

bool
StmArrayStatu2::SetBranch( intt    		 	 in_nBranchIx,
						   StmStatu2::Branch in_nBranch )
{
	if ( csv::in_range(intt(0), in_nBranchIx, intt(nNrOfBranches) ) )
	{
		dpBranches[in_nBranchIx] = in_nBranch;
		return true;
	}
	return false;
}


StmStatu2::Branch
StmArrayStatu2::NextBy(intt in_nIndex) const
{
	if (in_nIndex < 0)
		throw X_AutodocParser(X_AutodocParser::x_InvalidChar);

	return in_nIndex < nNrOfBranches
				?	dpBranches[in_nIndex]
				:   dpBranches[nNrOfBranches - 1];
}


bool
StmArrayStatu2::IsADefault() const
{
	return bIsADefault;
}

StmArrayStatu2 *
StmArrayStatu2::AsArray() 
{
	return this;
}

