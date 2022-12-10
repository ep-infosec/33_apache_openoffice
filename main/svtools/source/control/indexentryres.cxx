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




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/svtdata.hxx>
#include <svtools/svtools.hrc>
#include <svtools/indexentryres.hxx>

// -------------------------------------------------------------------------
//
//  wrapper for locale specific translations data of indexentry algorithm
//
// -------------------------------------------------------------------------

class IndexEntryRessourceData
{
	friend class IndexEntryRessource;
	private: /* data */
		String 	ma_Name;
		String 	ma_Translation;
	private: /* member functions */
		IndexEntryRessourceData () {}
	public:
		IndexEntryRessourceData ( const String &r_Algorithm, const String &r_Translation)
				: ma_Name (r_Algorithm), ma_Translation (r_Translation) {}

		const String&	GetAlgorithm () const { return ma_Name; }

		const String&	GetTranslation () const { return ma_Translation; }

		~IndexEntryRessourceData () {}

		IndexEntryRessourceData& operator= (const IndexEntryRessourceData& r_From)
		{
			ma_Name 		= r_From.GetAlgorithm();
			ma_Translation 	= r_From.GetTranslation();
			return *this;
		}
};

// -------------------------------------------------------------------------
//
//  implementation of the indexentry-algorithm-name translation
//
// -------------------------------------------------------------------------

#define INDEXENTRY_RESSOURCE_COUNT (STR_SVT_INDEXENTRY_END - STR_SVT_INDEXENTRY_START + 1)

IndexEntryRessource::IndexEntryRessource()
{
        mp_Data = new IndexEntryRessourceData[INDEXENTRY_RESSOURCE_COUNT];

        #define ASCSTR(str) String(RTL_CONSTASCII_USTRINGPARAM(str))
        #define RESSTR(rid) String(SvtResId(rid))

        mp_Data[STR_SVT_INDEXENTRY_ALPHANUMERIC - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("alphanumeric"), RESSTR(STR_SVT_INDEXENTRY_ALPHANUMERIC));
        mp_Data[STR_SVT_INDEXENTRY_DICTIONARY - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("dict"), RESSTR(STR_SVT_INDEXENTRY_DICTIONARY));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("pinyin"), RESSTR(STR_SVT_INDEXENTRY_PINYIN));
        mp_Data[STR_SVT_INDEXENTRY_PINYIN - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("radical"), RESSTR(STR_SVT_INDEXENTRY_RADICAL));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("stroke"), RESSTR(STR_SVT_INDEXENTRY_STROKE));
        mp_Data[STR_SVT_INDEXENTRY_STROKE - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("zhuyin"), RESSTR(STR_SVT_INDEXENTRY_ZHUYIN));
        mp_Data[STR_SVT_INDEXENTRY_ZHUYIN - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric first) (grouped by syllable)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FS - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric first) (grouped by consonant)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_FC));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_FC - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric last) (grouped by syllable)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LS));
        mp_Data[STR_SVT_INDEXENTRY_PHONETIC_LS - STR_SVT_INDEXENTRY_START] =
		IndexEntryRessourceData (ASCSTR("phonetic (alphanumeric last) (grouped by consonant)"),
                    RESSTR(STR_SVT_INDEXENTRY_PHONETIC_LC));
}

IndexEntryRessource::~IndexEntryRessource()
{
	delete[] mp_Data;
}

const String&
IndexEntryRessource::GetTranslation (const String &r_Algorithm)
{
	xub_StrLen nIndex = r_Algorithm.Search('.');
	String aLocaleFreeAlgorithm;

	if (nIndex == STRING_NOTFOUND)
		aLocaleFreeAlgorithm = r_Algorithm;
	else {
		nIndex += 1;
		aLocaleFreeAlgorithm = String(r_Algorithm, nIndex, r_Algorithm.Len() - nIndex);
	}

	for (sal_uInt32 i = 0; i < INDEXENTRY_RESSOURCE_COUNT; i++)
		if (aLocaleFreeAlgorithm == mp_Data[i].GetAlgorithm())
			return mp_Data[i].GetTranslation();
	return r_Algorithm;
}

