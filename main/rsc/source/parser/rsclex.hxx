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


#include <tools/stack.hxx>

#include <hash_set>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>

// a buffer for unique strings
class StringContainer
{
    std::hash_set< rtl::OString, rtl::OStringHash >		m_aStrings;
public:
    StringContainer() {}
    ~StringContainer() {}

    const char* putString( const char* pString );
};


enum MODE_ENUM { MODE_MODELESS, MODE_APPLICATIONMODAL, MODE_SYSTEMMODAL };

enum JUSTIFY_ENUM { JUST_CENTER, JUST_RIGHT, JUST_LEFT };

enum SHOW_ENUM { SHOW_NORMAL, SHOW_MINIMIZED, SHOW_MAXIMIZED };

enum ENUMHEADER { HEADER_NAME, HEADER_NUMBER };

enum REF_ENUM { TYPE_NOTHING, TYPE_REF, TYPE_COPY };

struct RSCHEADER {
    RscTop *    pClass;
    RscExpType  nName1;
    REF_ENUM    nTyp;
    RscTop *    pRefClass;
    RscExpType  nName2;
};

/************** O b j e c t s t a c k ************************************/
struct Node {
    Node*   pPrev;
    RSCINST aInst;
    sal_uInt32  nTupelRec;  // Rekursionstiefe fuer Tupel
    Node() { pPrev = NULL; nTupelRec = 0; };
};

class ObjectStack {
    private :
        Node* pRoot;
    public :

        ObjectStack ()   { pRoot = NULL; }

        const RSCINST & Top  ()     { return pRoot->aInst; }
        sal_Bool        IsEmpty()   { return( pRoot == NULL ); }
		void		IncTupelRec() { pRoot->nTupelRec++; }
		void		DecTupelRec() { pRoot->nTupelRec--; }
		sal_uInt32	TupelRecCount() const { return pRoot->nTupelRec; }
        void        Push( RSCINST aInst )
                    {
                        Node* pTmp;

                        pTmp         = pRoot;
                        pRoot        = new Node;
                        pRoot->aInst = aInst;
                        pRoot->pPrev = pTmp;
                    }
        void        Pop()
                    {
                       Node* pTmp;

                       pTmp  = pRoot;
                       pRoot = pTmp->pPrev;
                       delete pTmp;
                    }
};

/****************** F o r w a r d s **************************************/
#if defined( RS6000 )
extern "C" int yyparse();   // forward Deklaration fuer erzeugte Funktion
extern "C" void yyerror( char * );
extern "C" int  yylex( void );
#elif defined( HP9000 ) || defined( SCO ) || defined ( SOLARIS )
extern "C" int yyparse();   // forward Deklaration fuer erzeugte Funktion
extern "C" void yyerror( const char * );
extern "C" int  yylex( void );
#else
int yyparse();              // forward declaration for generated function
void yyerror( char * );
int  yylex( void );
#endif

class RscTypCont;
class RscFileInst;

extern RscTypCont*              pTC;
extern RscFileInst *            pFI;
extern RscExpression *          pExp;
extern ObjectStack              S;
extern StringContainer*			pStringContainer;
