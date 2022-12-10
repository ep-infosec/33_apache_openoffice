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
#include "precompiled_sw.hxx"


#include <wrtsh.hxx>
#include <crsskip.hxx>


/*
* private Methoden, die den Cursor ueber Suchen bewegen. Das
* Aufheben der Selektion muss auf der Ebene darueber erfolgen.
*/

/*
* Der Anfang eines Wortes ist das Folgen eines nicht-
* Trennzeichens auf Trennzeichen. Ferner das Folgen von
* nicht-Satztrennern auf Satztrenner. Der Absatzanfang ist
* ebenfalls Wortanfang.
*/


sal_Bool SwWrtShell::_SttWrd()
{
	if ( IsSttPara() )
		return 1;
		/*
			* temporaeren Cursor ohne Selektion erzeugen
			*/
	Push();
	ClearMark();
	if( !GoStartWord() )
			// nicht gefunden --> an den Absatzanfang
		SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
	ClearMark();
		// falls vorher Mark gesetzt war, zusammenfassen
	Combine();
	return 1;
}
/*
 * Das Ende eines Wortes ist das Folgen von Trennzeichen auf
 * nicht-Trennzeichen.	Unter dem Ende eines Wortes wird
 * ebenfalls die Folge von Worttrennzeichen auf Interpunktions-
 * zeichen verstanden. Das Absatzende ist ebenfalls Wortende.
 */



sal_Bool SwWrtShell::_EndWrd()
{
	if ( IsEndWrd() )
		return 1;
		// temporaeren Cursor ohne Selektion erzeugen
	Push();
	ClearMark();
	if( !GoEndWord() )
			// nicht gefunden --> an das Absatz Ende
		SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);
	ClearMark();
		// falls vorher Mark gesetzt war, zusammenfassen
	Combine();
	return 1;
}



sal_Bool SwWrtShell::_NxtWrd()
{
    sal_Bool bRet = sal_False;
	while( IsEndPara() )				// wenn schon am Ende, dann naechsten ???
	{
		if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))	// Document - Ende ??
		{
			Pop( sal_False );
			return bRet;
		}
        bRet = IsStartWord();
	}
	Push();
	ClearMark();
    while( !bRet )
    {
        if( !GoNextWord() )
        {
            if( (!IsEndPara() && !SwCrsrShell::MovePara( fnParaCurr, fnParaEnd ) )
                || !SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = sal_True;
    }
	ClearMark();
	Combine();
	return bRet;
}

sal_Bool SwWrtShell::_PrvWrd()
{
    sal_Bool bRet = sal_False;
	while( IsSttPara() )
	{								// wenn schon am Anfang, dann naechsten ???
		if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
		{							// Document - Anfang ??
			Pop( sal_False );
			return bRet;
		}
		bRet = IsStartWord();
	}
	Push();
	ClearMark();
    while( !bRet )
    {
        if( !GoPrevWord() )
        {
            if( (!IsSttPara() && !SwCrsrShell::MovePara( fnParaCurr, fnParaStart ) )
                || !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = sal_True;
    }
	ClearMark();
	Combine();
	return bRet;
}

// --> OD 2008-08-06 #i92468#
// method code of <SwWrtShell::_NxtWrd()> before fix for issue i72162
sal_Bool SwWrtShell::_NxtWrdForDelete()
{
    if ( IsEndPara() )
    {
        if ( !SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
        {
            Pop( sal_False );
            return sal_False;
        }
        return sal_True;
    }
    Push();
    ClearMark();
    if ( !GoNextWord() )
    {
        SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
    }
    ClearMark();
    Combine();
    return sal_True;
}

// method code of <SwWrtShell::_PrvWrd()> before fix for issue i72162
sal_Bool SwWrtShell::_PrvWrdForDelete()
{
    if ( IsSttPara() )
    {
        if ( !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
        {
            Pop( sal_False );
            return sal_False;
        }
        return sal_True;
    }
    Push();
    ClearMark();
    if( !GoPrevWord() )
    {
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    }
    ClearMark();
    Combine();
    return sal_True;
}
// <--


sal_Bool SwWrtShell::_FwdSentence()
{
	Push();
	ClearMark();
	if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
	{
		Pop(sal_False);
		return 0;
	}
	if( !GoNextSentence() && !IsEndPara() )
		SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);

	ClearMark();
	Combine();
	return 1;
}



sal_Bool SwWrtShell::_BwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
    {
        Pop(sal_False);
        return sal_False;
    }
    if(IsSttPara())
    {
        Pop();
        return sal_True;
    }
    if( !GoPrevSentence()  && !IsSttPara() )
        // nicht gefunden --> an den Absatz Anfang
            SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
    Combine();

    return sal_True;
}


sal_Bool SwWrtShell::_FwdPara()
{
	Push();
	ClearMark();
    // --> OD 2009-01-06 #i81824#
    // going right and back again left not needed and causes too much
    // accessibility events due to the cursor movements.
//    if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
//    {
//        Pop(sal_False);
//        return 0;
//    }
//    SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
    // <--
	sal_Bool bRet = SwCrsrShell::MovePara(fnParaNext, fnParaStart);

	ClearMark();
	Combine();
	return bRet;
}


sal_Bool SwWrtShell::_BwdPara()
{
	Push();
	ClearMark();
    // --> OD 2009-01-06 #i81824#
    // going left and back again right not needed and causes too much
    // accessibility events due to the cursor movements.
//    if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
//    {
//        Pop(sal_False);
//        return 0;
//    }
//    SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
    // <--
    // --> OD 2009-01-06 #i81824#
    // going to start of paragraph only needed, if move to previous paragraph
    // does not happen. Otherwise, useless accessibility events are triggered
    // due to cursor movements.
//    if(!IsSttOfPara())
//        SttPara();
	sal_Bool bRet = SwCrsrShell::MovePara(fnParaPrev, fnParaStart);
    if ( !bRet && !IsSttOfPara() )
    {
        SttPara();
    }
    // <--

	ClearMark();
	Combine();
	return bRet;
}


