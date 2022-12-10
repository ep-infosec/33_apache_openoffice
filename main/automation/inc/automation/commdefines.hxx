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



/*************************************************************************
 *
 *	  ATTENTION
 *	  This file is intended to work inside and outside the OpenOffice environment.
 *	  Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/

#include <automation/commtypes.hxx>
#include <osl/endian.h>

#ifndef MAKEDWORD
#define MAKEDWORD(wl, wh)   ((comm_DWORD)((wl) & 0xFFFF) | (((comm_DWORD)(wh) & 0xFFFF) << 16))
#endif
#ifndef LOWORD
#define LOWORD(d)           ((comm_WORD)((comm_DWORD)(d) & 0xFFFF))
#endif
#ifndef HIWORD
#define HIWORD(d)           ((comm_WORD)(((comm_DWORD)(d) >> 16) & 0xFFFF))
#endif
#ifndef MAKEWORD
#define MAKEWORD(bl, bh)    ((comm_WORD)((bl) & 0xFF) | (((comm_WORD)(bh) & 0xFF) << 8))
#endif
#ifndef LOBYTE
#define LOBYTE(w)           ((comm_BYTE)((comm_WORD)(w) & 0xFF))
#endif
#ifndef HIBYTE
#define HIBYTE(w)           ((comm_BYTE)(((comm_WORD)(w) >> 8) & 0xFF))
#endif
#ifndef MAKEBYTE
#define MAKEBYTE(nl, nh)    ((comm_BYTE)(((nl) & 0x0F) | (((nh) & 0x0F) << 4)))
#endif
#ifndef LONIBBLE
#define LONIBBLE(b)         ((comm_BYTE)((b) & 0x0F))
#endif
#ifndef HINIBBLE
#define HINIBBLE(b)         ((comm_BYTE)(((b) >> 4) & 0x0F))
#endif

#ifndef SWAPWORD
#define SWAPWORD(w)         MAKEWORD(HIBYTE(w),LOBYTE(w))
#endif
#ifndef SWAPDWORD
#define SWAPDWORD(d)        MAKEDWORD(SWAPWORD(HIWORD(d)),SWAPWORD(LOWORD(d)))
#endif

#ifdef OSL_BIGENDIAN
#ifndef NETWORD
#define NETWORD(w)          (comm_WORD)(w)
#endif
#ifndef NETDWORD
#define NETDWORD(d)         (comm_DWORD)(d)
#endif
#endif // OSL_BIGENDIAN

#ifdef OSL_LITENDIAN
#ifndef NETWORD
#define NETWORD(w)          MAKEWORD(HIBYTE(w),LOBYTE(w))
#endif
#ifndef NETDWORD
#define NETDWORD(d)         MAKEDWORD(NETWORD(HIWORD(d)),NETWORD(LOWORD(d)))
#endif
#endif // OSL_LITENDIAN

/**
	Es gibt zwei Arten von Datenpaketen
	Die erste enth�lt in den ersten 4 Byte die L�ngenangabe und in den darauffolgenden die Daten.
	Die L�ngenangabe bezieht sich nur auf die Daten ohne die L�ngenangabe selbst.

	Die zweite Art von Datenpaketen enth�lt Header mit weiteren Informationen
	wie unten beschrieben.

	Umgeschaltet wird mit dem Boolean bUseMultiChannel im Konstruktor des Managers.
**/
/**
Defines f�r Header Typen:

Allgemeiner Header:
		Byte	L�nge		Inhalt
		0..3		4		L�nge des Paketes ohne diese 4 Byte
		4			1		Pr�fsumme �ber die L�nge. Stimmt sie nicht wird die Verbindung geschlossen
		5..6		2		L�nge des Headers ohne diese 2 Byte
		7..8		2		Typ des Headers

CH_SimpleMultiChannel:
		9..10		2		Channel
CH_Handshake				Internal Use ONLY
							Keine Weiteren Daten!

**/
typedef comm_UINT16 CMProtocol;

#define CM_PROTOCOL_OLDSTYLE		(CMProtocol)0x0001
#define CM_PROTOCOL_MARS			(CMProtocol)0x0001
#define CM_PROTOCOL_BROADCASTER		(CMProtocol)0x0002
#define CM_PROTOCOL_USER_START		(CMProtocol)0x0100

typedef comm_USHORT HandshakeType;
typedef comm_USHORT CommunicationOption;

#define CH_NoHeader					0x0000
#define CH_SimpleMultiChannel		0x0001
#define CH_Handshake				0x0002

#define CH_REQUEST_HandshakeAlive	((HandshakeType)0x0101)	/// Fordert eine Alive Antwort an
#define CH_RESPONSE_HandshakeAlive	((HandshakeType)0x0102)	/// Alive Antwort

/**
	Announce supported options:
	Client announces available options
	Server returns subset of these options (note that the sbset can be the entire set also)
**/
#define CH_SUPPORT_OPTIONS			((HandshakeType)0x0103)
#define OPT_USE_SHUTDOWN_PROTOCOL	((CommunicationOption)0x0001)

/// these are for making sure all Data is read prior to shutting down the link
#define CH_REQUEST_ShutdownLink		((HandshakeType)0x0104)	/// Request to Shutdown this link
#define CH_ShutdownLink				((HandshakeType)0x0105)	/// Shutdown this link

#define CH_SetApplication			((HandshakeType)0x0106)	/// Set Description of Client

