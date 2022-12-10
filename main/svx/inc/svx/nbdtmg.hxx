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

#ifndef _NBDTMG_HXX
#define _NBDTMG_HXX
#include "svx/svxdllapi.h"

#ifndef _SVX_NUMITEM_HXX
#include <editeng/numitem.hxx>
#endif
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <svl/svarray.hxx>
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

namespace svx { namespace sidebar {

#define DEFAULT_BULLET_TYPES							8
#define DEFAULT_NONE									10
#define DEFAULT_NUM_TYPE_MEMBER							5 
#define DEFAULT_NUM_VALUSET_COUNT						8
#define DEFAULT_NUMBERING_CACHE_FORMAT_VERSION			0x10

typedef sal_uInt16 NBOType;
namespace eNBOType
{
    const NBOType BULLETS = 0x01;
    const NBOType GRAPHICBULLETS = 0x02;
    const NBOType NUMBERING = 0x03;
    const NBOType OUTLINE = 0x04;
    const NBOType MIXBULLETS = 0x05;
}

typedef sal_uInt16 NBType;
namespace eNBType
{
    const NBOType BULLETS = 0x01;
    const NBOType GRAPHICBULLETS = 0x02;
}

class  SVX_DLLPUBLIC NumSettings_Impl
{
	public:
		short		nNumberType;
		short 		nParentNumbering;
		SvxNumberFormat::SvxNumLabelFollowedBy eLabelFollowedBy;
		long 		nTabValue;
		SvxAdjust	eNumAlign;
		long			nNumAlignAt;
		long			nNumIndentAt;
		rtl::OUString	sPrefix;
		rtl::OUString	sSuffix;
		rtl::OUString	sBulletChar;
		rtl::OUString	sBulletFont;
		SvxBrushItem   *pBrushItem;
		Size			aSize;

	public:
		NumSettings_Impl() :
		  nNumberType(0),
		  nParentNumbering(0),
		  pBrushItem(0),
		  aSize(0,0)
			{}
		~NumSettings_Impl(){}
};
/*
typedef NumSettings_Impl* NumSettings_ImplPtr;
SV_DECL_PTRARR_DEL(NumSettingsArr_Impl,NumSettings_ImplPtr,8,4)
SV_IMPL_PTRARR( NumSettingsArr_Impl, NumSettings_ImplPtr )*/

class NumSettingsArr_Impl;

class  SVX_DLLPUBLIC BulletsSettings
{
	public:
		sal_Bool 		bIsCustomized;
		rtl::OUString	sDescription;
		NBType		eType;		
	public:
		BulletsSettings(NBType eTy) :
			bIsCustomized(sal_False),			
			eType(eTy)
			{}
		virtual ~BulletsSettings(){}
};

class  SVX_DLLPUBLIC BulletsSettings_Impl:public BulletsSettings
{
	public:		
		sal_Unicode	cBulletChar;
		//rtl::OUString	sBulletFont;
		Font			aFont;
		
	public:
		BulletsSettings_Impl(NBType eTy) :
			BulletsSettings(eTy)			
			{}
		virtual ~BulletsSettings_Impl(){}
};

class  SVX_DLLPUBLIC GrfBulDataRelation: public BulletsSettings
{
	public:
		String 	sGrfName;
		sal_uInt16	nTabIndex;
		sal_uInt16	nGallaryIndex;
		const Graphic*	pGrfObj;
		Size aSize;
	GrfBulDataRelation(NBType eTy):
		BulletsSettings(eTy),
		nTabIndex((sal_uInt16)0xFFFF),
		nGallaryIndex((sal_uInt16)0xFFFF),
		pGrfObj(0),
		aSize(0,0)
	{}
	virtual ~GrfBulDataRelation(){}
};

class  SVX_DLLPUBLIC MixBulletsSettings_Impl
{
	public:
		NBType			eType;
		sal_uInt16			nIndex; //index in the tab page display
		sal_uInt16			nIndexDefault;
		BulletsSettings*	pBullets;		
	public:
		MixBulletsSettings_Impl(NBType eTy) :			
			eType(eTy),
			nIndex((sal_uInt16)0xFFFF),
			nIndexDefault((sal_uInt16)0xFFFF),
			pBullets(0)
			{}
		~MixBulletsSettings_Impl(){}
};

class  SVX_DLLPUBLIC NumberSettings_Impl
{
	public:
		sal_Bool 		bIsCustomized;
		rtl::OUString	sDescription;
		sal_uInt16		nIndex; //index in the tab page display
		sal_uInt16		nIndexDefault;
		NumSettings_Impl	*pNumSetting;
	public:
		NumberSettings_Impl() :
			bIsCustomized(sal_False),
			nIndex((sal_uInt16)0xFFFF),
			nIndexDefault((sal_uInt16)0xFFFF),
			pNumSetting(NULL)
			{}
		~NumberSettings_Impl(){}
};

class NumberSettingsArr_Impl;

class  SVX_DLLPUBLIC OutlineSettings_Impl
{
	public:
		sal_Bool 		bIsCustomized;
		rtl::OUString	sDescription;
		NumSettingsArr_Impl	*pNumSettingsArr;
	public:
		OutlineSettings_Impl() :
			bIsCustomized(sal_False),
			pNumSettingsArr(NULL)
			{}
		~OutlineSettings_Impl(){
		}
};

class SVX_DLLPUBLIC NBOTypeMgrBase
{
	public:
		NBOType 		eType;
	private:
		const SfxItemSet*	pSet;
		SfxMapUnit		eCoreUnit;
		// store the attributes passed from pSet
		String			aNumCharFmtName;
		void			StoreBulCharFmtName_impl();
		void			StoreMapUnit_impl();

	public:
		NBOTypeMgrBase(const NBOType aType):eType(aType),pSet(0),eCoreUnit(SFX_MAPUNIT_TWIP),aNumCharFmtName(String()){}
		NBOTypeMgrBase(const NBOType aType,const SfxItemSet* pArg):eType(aType),pSet(pArg),eCoreUnit(SFX_MAPUNIT_TWIP),aNumCharFmtName(String()){}
		NBOTypeMgrBase(const NBOTypeMgrBase& aTypeMgr){eType = aTypeMgr.eType;pSet = aTypeMgr.pSet;eCoreUnit = aTypeMgr.eCoreUnit;aNumCharFmtName = aTypeMgr.aNumCharFmtName; }
		virtual ~NBOTypeMgrBase() {}	
		virtual void Init()=0;
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0)=0;
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF)=0;
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF, sal_Bool isDefault=false,sal_Bool isResetSize=false)=0;
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false)=0;
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex)=0;
		sal_uInt16 IsSingleLevel(sal_uInt16 nCurLevel);
		const SfxItemSet* GetItems() { return pSet;}
		// store the attributes passed from pSet
		void SetItems(const SfxItemSet* pArg) { pSet = pArg;StoreBulCharFmtName_impl();StoreMapUnit_impl();}
	protected:
		String GetBulCharFmtName();
		SfxMapUnit GetMapUnit();
	protected:
		sal_Bool	bIsLoading;
		void	ImplLoad(String filename);
		void	ImplStore(String filename);
		
};


class SVX_DLLPUBLIC BulletsTypeMgr: public NBOTypeMgrBase
{
	friend class OutlineTypeMgr;
	friend class NumberingTypeMgr;
	public:
		static sal_Unicode aDynamicBulletTypes[DEFAULT_BULLET_TYPES];
		static sal_Unicode aDynamicRTLBulletTypes[DEFAULT_BULLET_TYPES];
		static BulletsSettings_Impl* pActualBullets[DEFAULT_BULLET_TYPES];
		static BulletsTypeMgr* _instance;
	public:		
		BulletsTypeMgr(const NBOType aType);
		BulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg);
		BulletsTypeMgr(const BulletsTypeMgr& aTypeMgr);
		virtual ~BulletsTypeMgr() {}
		virtual void Init();			
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);	
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
		sal_Unicode GetBulChar(sal_uInt16 nIndex);
		Font GetBulCharFont(sal_uInt16 nIndex);
		static BulletsTypeMgr* GetInstance()
		{
			 if ( _instance == 0 )
	                {
	                    _instance = new BulletsTypeMgr(eNBOType::BULLETS);
	                }

	                return _instance;
		}
		
};

class SVX_DLLPUBLIC GraphyicBulletsTypeMgr: public NBOTypeMgrBase
{
	friend class OutlineTypeMgr;
	friend class NumberingTypeMgr;
	public:
		List	aGrfDataLst;
		static GraphyicBulletsTypeMgr* _instance;
	public:		
		GraphyicBulletsTypeMgr(const NBOType aType);
		GraphyicBulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg);
		GraphyicBulletsTypeMgr(const GraphyicBulletsTypeMgr& aTypeMgr);
		virtual ~GraphyicBulletsTypeMgr() {}
		virtual void Init();			
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);	
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
		String GetGrfName(sal_uInt16 nIndex);
		static GraphyicBulletsTypeMgr* GetInstance()
		{
			 if ( _instance == 0 )
	                {
	                    _instance = new GraphyicBulletsTypeMgr(eNBOType::BULLETS);
	                }

	                return _instance;
		}
		
};

class SVX_DLLPUBLIC MixBulletsTypeMgr: public NBOTypeMgrBase
{
	friend class OutlineTypeMgr;
	friend class NumberingTypeMgr;
	public:		
		static MixBulletsSettings_Impl* pActualBullets[DEFAULT_BULLET_TYPES];
		static MixBulletsSettings_Impl* pDefaultActualBullets[DEFAULT_BULLET_TYPES];
		static MixBulletsTypeMgr* _instance;
	public:		
		MixBulletsTypeMgr(const NBOType aType);
		MixBulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg);
		MixBulletsTypeMgr(const MixBulletsTypeMgr& aTypeMgr);
		virtual ~MixBulletsTypeMgr() {}
		virtual void Init();			
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);	
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex);		
		static MixBulletsTypeMgr* GetInstance()
		{
			 if ( _instance == 0 )
	                {
	                    _instance = new MixBulletsTypeMgr(eNBOType::MIXBULLETS);
	                }

	                return _instance;
		}
		
};

class SVX_DLLPUBLIC NumberingTypeMgr: public NBOTypeMgrBase
{
	public:
		//NumSettingsArr_Impl*		pNumSettingsArr;
		NumberSettingsArr_Impl*		pNumberSettingsArr;
		NumberSettingsArr_Impl*		pDefaultNumberSettingsArr;
		static NumberingTypeMgr* 	_instance;
	public:		
		NumberingTypeMgr(const NBOType aType);
		NumberingTypeMgr(const NBOType aType,const SfxItemSet* pArg);
		NumberingTypeMgr(const NumberingTypeMgr& aTypeMgr);
		virtual ~NumberingTypeMgr() {}
		virtual void Init();
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);	
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
		sal_uInt16 GetNumCount() const;
		NumberSettings_Impl* GetNumSettingByIndex(sal_uInt16 nIndex) const;
		static NumberingTypeMgr* GetInstance()
		{
			 if ( _instance == 0 )
	                {
	                    _instance = new NumberingTypeMgr(eNBOType::NUMBERING);
	                }

	                return _instance;
		}
};

class SVX_DLLPUBLIC OutlineTypeMgr: public NBOTypeMgrBase
{
	public:
		//NumSettingsArr_Impl*		pNumSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
		OutlineSettings_Impl*		pOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
		OutlineSettings_Impl*		pDefaultOutlineSettingsArrs[DEFAULT_NUM_VALUSET_COUNT];
		static OutlineTypeMgr* 	_instance;
	public:		
		OutlineTypeMgr(const NBOType aType);
		OutlineTypeMgr(const NBOType aType,const SfxItemSet* pArg);
		OutlineTypeMgr(const OutlineTypeMgr& aTypeMgr);
		virtual ~OutlineTypeMgr() {}
		virtual void Init();
		virtual sal_uInt16 GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex=0);
		virtual sal_Bool RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF);
		virtual sal_Bool ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel=(sal_uInt16)0xFFFF,sal_Bool isDefault=false,sal_Bool isResetSize=false);	
		virtual String GetDescription(sal_uInt16 nIndex,sal_Bool isDefault=false);
		virtual sal_Bool IsCustomized(sal_uInt16 nIndex);
		static OutlineTypeMgr* GetInstance()
		{
			 if ( _instance == 0 )
	                {
	                    _instance = new OutlineTypeMgr(eNBOType::OUTLINE);
	                }

	                return _instance;
		}
};
}}
#endif

