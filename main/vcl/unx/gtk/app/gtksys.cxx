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
#include "precompiled_vcl.hxx"

#include <unx/svunx.h>
#include <svdata.hxx>
#include <vcl/window.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <cstdio>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

SalSystem *GtkInstance::CreateSalSystem()
{
		return new GtkSalSystem();
}

GtkSalSystem::~GtkSalSystem()
{
}

int GtkSalSystem::ShowNativeDialog( const String& rTitle,
									const String& rMessage,
									const std::list< String >& rButtons,
									int nDefButton )
{

	ImplSVData* pSVData = ImplGetSVData();
	if( pSVData->mpIntroWindow )
			pSVData->mpIntroWindow->Hide();

#if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "GtkSalSystem::ShowNativeDialog\n");
#endif

	ByteString aTitle( rTitle, RTL_TEXTENCODING_UTF8 );
	ByteString aMessage( rMessage, RTL_TEXTENCODING_UTF8 );

    /* Create the dialogue */
    GtkWidget* mainwin = gtk_message_dialog_new
			( NULL, (GtkDialogFlags)0, GTK_MESSAGE_WARNING,
			  GTK_BUTTONS_NONE, aMessage.GetBuffer(), NULL );
    gtk_window_set_title( GTK_WINDOW( mainwin ), aTitle.GetBuffer() );

    gint nButtons = 0, nResponse;

	int nButton = 0;
	for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
	{
        ByteString aLabel( *it, RTL_TEXTENCODING_UTF8 );

		if( nButton == nDefButton )
		{
            gtk_dialog_add_button( GTK_DIALOG( mainwin ), aLabel.GetBuffer(), nButtons );
            gtk_dialog_set_default_response( GTK_DIALOG( mainwin ), nButtons );
		}
		else
			gtk_dialog_add_button( GTK_DIALOG( mainwin ), aLabel.GetBuffer(), nButtons );
		nButtons++;
	}

    nResponse = gtk_dialog_run( GTK_DIALOG(mainwin) );
    if( nResponse == GTK_RESPONSE_NONE || nResponse == GTK_RESPONSE_DELETE_EVENT )
        nResponse = -1;

    gtk_widget_destroy( GTK_WIDGET(mainwin) );

	return nResponse;
}
