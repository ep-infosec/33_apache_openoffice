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



#ifndef SC_FUNKTION_H
#define SC_FUNKTION_H

// Bearbeitungsfunktionen
void P0( void );	// 0 Parameter
void P1( void );	// 1 Parameter
void P2( void );	// 2 Parameter
void P3( void );	// 3 Parameter
void P4( void );	// 4 Parameter
void P5( void );	// 5 Parameter
void Pn( void );	// n Parameter
void NI( void );	// nicht implementiert
void ConstFloat( void );	//	  0
void Variable( void );		//	  1
void LotusRange( void );	//    2
void FormulaReturn( void );	//    3
void Klammer( void );		//	  4
void ConstInt( void );		//    5
void ConstString( void );	//    6
// ACHTUNG: unbekannte Funktionen -> P0()

#endif

