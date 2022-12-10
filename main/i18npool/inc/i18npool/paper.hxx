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



#ifndef INCLUDED_I18NPOOL_PAPER_HXX
#define INCLUDED_I18NPOOL_PAPER_HXX

#include <sal/config.h>

#include "i18npool/i18npooldllapi.h"
#include <rtl/string.hxx>
#include <com/sun/star/lang/Locale.hpp>

//!! WARNING: be aware of cui/source/tabpages/page.h where someone had the broken idea
//!! of duplicating the values of this enum in order to use those defines within
//!! src files.
//!! Thus don't reorder the enum values here without changing the code there as well.
enum Paper
{
	PAPER_A0,
	PAPER_A1,
	PAPER_A2,
	PAPER_A3,
	PAPER_A4,
	PAPER_A5,
	PAPER_B4_ISO,
	PAPER_B5_ISO,
	PAPER_LETTER,
	PAPER_LEGAL,
	PAPER_TABLOID,
	PAPER_USER,
	PAPER_B6_ISO,
	PAPER_ENV_C4,
	PAPER_ENV_C5,
	PAPER_ENV_C6,
	PAPER_ENV_C65,
	PAPER_ENV_DL,
	PAPER_SLIDE_DIA,
	PAPER_SCREEN_4_BY_3,
	PAPER_C,
	PAPER_D,
	PAPER_E,
	PAPER_EXECUTIVE,
	PAPER_FANFOLD_LEGAL_DE,
	PAPER_ENV_MONARCH,
	PAPER_ENV_PERSONAL,
	PAPER_ENV_9,
	PAPER_ENV_10,
	PAPER_ENV_11,
	PAPER_ENV_12,
	PAPER_KAI16,
	PAPER_KAI32,
	PAPER_KAI32BIG,
	PAPER_B4_JIS,
	PAPER_B5_JIS,
	PAPER_B6_JIS,
	PAPER_LEDGER,
	PAPER_STATEMENT,
	PAPER_QUARTO,
	PAPER_10x14,
	PAPER_ENV_14,
	PAPER_ENV_C3,
	PAPER_ENV_ITALY,
	PAPER_FANFOLD_US,
	PAPER_FANFOLD_DE,
	PAPER_POSTCARD_JP,
	PAPER_9x11,
	PAPER_10x11,
	PAPER_15x11,
	PAPER_ENV_INVITE,
	PAPER_A_PLUS,
	PAPER_B_PLUS,
	PAPER_LETTER_PLUS,
	PAPER_A4_PLUS,
	PAPER_DOUBLEPOSTCARD_JP,
	PAPER_A6,
	PAPER_12x11,
	PAPER_A7,
	PAPER_A8,
	PAPER_A9,
	PAPER_A10,
	PAPER_B0_ISO,
	PAPER_B1_ISO,
	PAPER_B2_ISO,
	PAPER_B3_ISO,
	PAPER_B7_ISO,
	PAPER_B8_ISO,
	PAPER_B9_ISO,
	PAPER_B10_ISO,
	PAPER_ENV_C2,
	PAPER_ENV_C7,
	PAPER_ENV_C8,
	PAPER_ARCHA,
	PAPER_ARCHB,
	PAPER_ARCHC,
	PAPER_ARCHD,
	PAPER_ARCHE,
	PAPER_SCREEN_16_BY_9,
	PAPER_SCREEN_16_BY_10
};

// defined for 'equal size' test with the implementation array
#define NUM_PAPER_ENTRIES	(PAPER_SCREEN_16_BY_10 - PAPER_A0 + 1)

// Paper

class I18NPOOL_DLLPUBLIC PaperInfo
{
	Paper m_eType;
	long m_nPaperWidth; // width in 100thMM
	long m_nPaperHeight; // height in 100thMM
public:
	PaperInfo(Paper eType);
	PaperInfo(long nPaperWidth, long nPaperHeight);

	Paper getPaper() const { return m_eType; }
	long getWidth() const { return m_nPaperWidth; }
	long getHeight() const { return m_nPaperHeight; }
	bool sloppyEqual(const PaperInfo &rOther) const;
	bool doSloppyFit();

	static PaperInfo getSystemDefaultPaper();
	static PaperInfo getDefaultPaperForLocale(const ::com::sun::star::lang::Locale & rLocale);

	static Paper fromPSName(const rtl::OString &rName);
	static rtl::OString toPSName(Paper eType);

	static long sloppyFitPageDimension(long nDimension);
};

#endif // INCLUDED_I18NPOOL_PAPER_HXX
