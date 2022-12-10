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



#ifndef _SGVSPLN_HXX
#define _SGVSPLN_HXX

/*************************************************************************
|*
|*    CalcSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines parametrischen
|*                      nat�rlichen oder periodischen kubischen
|*                      Polynomsplines. Die Eckpunkte des �bergebenen
|*                      Polygons werden als St�tzstellen angenommen.
|*                      n liefert die Anzahl der Teilpolynome.
|*                      Ist die Berechnung fehlerfrei verlaufen, so
|*                      liefert die Funktion sal_True. Nur in diesem Fall
|*                      ist Speicher f�r die Koeffizientenarrays
|*                      allokiert, der dann sp�ter vom Aufrufer mittels
|*                      delete freizugeben ist.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/

sal_Bool CalcSpline(Polygon& rPoly, sal_Bool Periodic, sal_uInt16& n,
				double*& ax, double*& ay, double*& bx, double*& by,
				double*& cx, double*& cy, double*& dx, double*& dy, double*& T);

/*************************************************************************
|*
|*    Poly2Spline()
|*
|*    Beschreibung      Konvertiert einen parametrichen kubischen
|*                      Polynomspline Spline (nat�rlich oder periodisch)
|*                      in ein angen�hertes Polygon.
|*                      Die Funktion liefert sal_False, wenn ein Fehler bei
|*                      der Koeffizientenberechnung aufgetreten ist oder
|*                      das Polygon zu gro?wird (>PolyMax=16380). Im 1.
|*                      Fall hat das Polygon 0, im 2. Fall PolyMax Punkte.
|*                      Um Koordinaten�berl�ufe zu vermeiden werden diese
|*                      auf +/-32000 begrenzt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool Spline2Poly(Polygon& rSpln, sal_Bool Periodic, Polygon& rPoly);

#endif //_SGVSPLN_HXX
