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



#include "precompiled_shell.hxx"
#include "sal/config.h"

#include <string.h>

#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/file.hxx"
#include "osl/security.hxx"
#include "osl/thread.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"

#include "gconfaccess.hxx"

#define GCONF_PROXY_MODE_KEY "/system/proxy/mode"
#define GCONF_AUTO_SAVE_KEY  "/apps/openoffice/auto_save"

namespace gconfaccess {

namespace {

namespace css = com::sun::star ;
namespace uno = css::uno ;
using namespace rtl;

GConfClient* getGconfClient()
{
    static GConfClient* mClient= 0;
    if (mClient == NULL)
    {
        /* initialize glib object type library */
        g_type_init();
	
        GError* aError = NULL;
        if (!gconf_init(0, NULL, &aError))
        {
            rtl::OUStringBuffer msg;
            msg.appendAscii("GconfBackend:GconfLayer: Cannot Initialize Gconf connection - " );
            msg.appendAscii(aError->message);
            
            g_error_free(aError); 
            aError = NULL;
            throw uno::RuntimeException(msg.makeStringAndClear(),NULL);
        }
	
        mClient = gconf_client_get_default();
        if (!mClient)
        {
            throw uno::RuntimeException(rtl::OUString::createFromAscii
                ("GconfBackend:GconfLayer: Cannot Initialize Gconf connection"),NULL);
        }

        static const char * const PreloadValuesList[] =
        {
            "/desktop/gnome/interface",
            "/system/proxy",
            "/system/http_proxy/host",
            "/desktop/gnome/url-handlers/mailto",
#ifdef ENABLE_LOCKDOWN
            "/apps/openoffice",
            "/desktop/gnome/lockdown",
            "/apps/openoffice/lockdown",
#endif // ENABLE_LOCKDOWN
            NULL
        };
        int i = 0;
        while( PreloadValuesList[i] != NULL )
            gconf_client_preload( mClient, PreloadValuesList[i++], GCONF_CLIENT_PRELOAD_ONELEVEL, NULL );
    } 

    return mClient;
}

static OUString xdg_user_dir_lookup (const char *type)
{
    char *config_home;
    char *p;
    int relative;
    bool bError = false;

    osl::Security aSecurity;
    oslFileHandle handle;
    OUString aHomeDirURL;
    OUString aDocumentsDirURL;
    OUString aConfigFileURL;
    OUStringBuffer aUserDirBuf;

    if (!aSecurity.getHomeDir( aHomeDirURL ) )
    {
	osl::FileBase::getFileURLFromSystemPath(rtl::OUString::createFromAscii("/tmp"), aDocumentsDirURL);
	return aDocumentsDirURL;
    }

    config_home = getenv ("XDG_CONFIG_HOME");
    if (config_home == NULL || config_home[0] == 0)
    {
	aConfigFileURL = OUString(aHomeDirURL);
	aConfigFileURL += OUString::createFromAscii( "/.config/user-dirs.dirs" );
    }
    else
    {
	aConfigFileURL = OUString::createFromAscii(config_home);
	aConfigFileURL += OUString::createFromAscii( "/user-dirs.dirs" );
    }

    if(osl_File_E_None == osl_openFile(aConfigFileURL.pData, &handle, osl_File_OpenFlag_Read))
    {
	rtl::ByteSequence seq;
	while (osl_File_E_None == osl_readLine(handle , (sal_Sequence **)&seq))
	{
	    /* Remove newline at end */
	    int len = seq.getLength();
	    if(len>0 && seq[len-1] == '\n')
		seq[len-1] = 0;

	    p = (char *)seq.getArray();

	    while (*p == ' ' || *p == '\t')
		p++;

	    if (strncmp (p, "XDG_", 4) != 0)
		continue;
	    p += 4;
	    if (strncmp (p, type, strlen (type)) != 0)
		continue;
	    p += strlen (type);
	    if (strncmp (p, "_DIR", 4) != 0)
		continue;
	    p += 4;

	    while (*p == ' ' || *p == '\t')
		p++;

	    if (*p != '=')
		continue;
	    p++;

	    while (*p == ' ' || *p == '\t')
		p++;

	    if (*p != '"')
		continue;
	    p++;

	    relative = 0;
	    if (strncmp (p, "$HOME/", 6) == 0)
	    {
		p += 6;
		relative = 1;
	    }
	    else if (*p != '/')
		continue;

	    if (relative)
	    {
		aUserDirBuf = OUStringBuffer(aHomeDirURL);
		aUserDirBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "/" ) );
	    }
	    else
	    {
		aUserDirBuf = OUStringBuffer();
	    }

	    while (*p && *p != '"')
	    {
		if ((*p == '\\') && (*(p+1) != 0))
		    p++;
		aUserDirBuf.append((sal_Unicode)*p++);
	    }
	}
      osl_closeFile(handle);
    }
    else
	bError = true;

    if (aUserDirBuf.getLength()>0 && !bError)
    {
	aDocumentsDirURL = aUserDirBuf.makeStringAndClear();
	osl::Directory aDocumentsDir( aDocumentsDirURL );
	if( osl::FileBase::E_None == aDocumentsDir.open() )
	    return aDocumentsDirURL;
    }

    /* Special case desktop for historical compatibility */
    if (strcmp (type, "DESKTOP") == 0)
    {
	aUserDirBuf = OUStringBuffer(aHomeDirURL);
	aUserDirBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "/Desktop" ) );
	return aUserDirBuf.makeStringAndClear();
    }
    else
    {
	aUserDirBuf = OUStringBuffer(aHomeDirURL);
	aUserDirBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "/Documents" ) );
	return aUserDirBuf.makeStringAndClear();
    }
}

//------------------------------------------------------------------------------

uno::Any makeAnyOfGconfValue( GConfValue *aGconfValue )
{
    switch( aGconfValue->type )
    {
        case GCONF_VALUE_BOOL:
            return uno::makeAny( (sal_Bool) gconf_value_get_bool( aGconfValue ) );

        case GCONF_VALUE_INT:
            return uno::makeAny( (sal_Int32) gconf_value_get_int( aGconfValue ) );

        case GCONF_VALUE_STRING:
            return uno::makeAny( OStringToOUString( rtl::OString(
                gconf_value_get_string(aGconfValue) ), RTL_TEXTENCODING_UTF8 ) );

        default:
            fprintf( stderr, "makeAnyOfGconfValue: Type not handled.\n" );
            break;
    }

    return uno::Any();
}

//------------------------------------------------------------------------------

static void splitFontName( GConfValue *aGconfValue, rtl::OUString &rName, sal_Int16 &rHeight)
{
   rtl::OString aFont( gconf_value_get_string( aGconfValue ) );
   aFont.trim();
   sal_Int32 nIdx = aFont.lastIndexOf( ' ' );
   if (nIdx < 1) { // urk
       rHeight = 12;
       nIdx = aFont.getLength();
   } else {
       rtl::OString aSize = aFont.copy( nIdx + 1 );
       rHeight = static_cast<sal_Int16>( aSize.toInt32() );
   }

   rName = rtl::OStringToOUString( aFont.copy( 0, nIdx ), RTL_TEXTENCODING_UTF8 );
}

//------------------------------------------------------------------------------

uno::Any translateToOOo( const ConfigurationValue aValue, GConfValue *aGconfValue )
{

    switch( aValue.nSettingId )
    {
        case SETTING_PROXY_MODE:
        {
            rtl::OUString aProxyMode;
            uno::Any aOriginalValue = makeAnyOfGconfValue( aGconfValue );
            aOriginalValue >>= aProxyMode;

            if( aProxyMode.equals( rtl::OUString::createFromAscii( "manual" ) ) )
                return uno::makeAny( (sal_Int32) 1 );
            else if( aProxyMode.equals( rtl::OUString::createFromAscii( "none" ) ) )
                return uno::makeAny( (sal_Int32) 0 );
        }
            break;

        case SETTING_NO_PROXY_FOR:
        {
            rtl::OStringBuffer aBuffer;
            if( (GCONF_VALUE_LIST == aGconfValue->type) && (GCONF_VALUE_STRING == gconf_value_get_list_type(aGconfValue)) )
            {
                GSList * list = gconf_value_get_list(aGconfValue);
                for(; list; list = g_slist_next(list))
                {
                    aBuffer.append(gconf_value_get_string((GConfValue *) list->data));
                    aBuffer.append(";");
                }
                // Remove trailing ";"
                aBuffer.setLength(aBuffer.getLength()-1);
                return uno::makeAny(rtl::OStringToOUString(aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
            }
            else
                g_warning( "unexpected type for ignore_hosts" );
        }
            break;
            
        case SETTING_MAILER_PROGRAM:
        {
            rtl::OUString aMailer;
            uno::Any aOriginalValue = makeAnyOfGconfValue( aGconfValue );
            aOriginalValue >>= aMailer;
            sal_Int32 nIndex = 0;
            return uno::makeAny( aMailer.getToken( 0, ' ', nIndex ) );
        }

#ifdef ENABLE_LOCKDOWN
        // "short" values need to be returned a sal_Int16
        case SETTING_FONT_ANTI_ALIASING_MIN_PIXEL:
        case SETTING_SYMBOL_SET:
        {
            sal_Int32 nShortValue;
            uno::Any aOriginalValue = makeAnyOfGconfValue( aGconfValue );
            aOriginalValue >>= nShortValue;
            return uno::makeAny( (sal_Int16) nShortValue );
        }
            break;
#endif // ENABLE_LOCKDOWN

        // "boolean" values that need a string to be returned
        case SETTING_ENABLE_ACCESSIBILITY:
#ifdef ENABLE_LOCKDOWN
        case SETTING_DISABLE_PRINTING:
#endif // ENABLE_LOCKDOWN
        {
            sal_Bool bBooleanValue = false;
            uno::Any aOriginalValue = makeAnyOfGconfValue( aGconfValue );
            aOriginalValue >>= bBooleanValue;
            return uno::makeAny( rtl::OUString::valueOf( (sal_Bool) bBooleanValue ) );
        }

        case SETTING_WORK_DIRECTORY:
        {
            rtl::OUString aDocumentsDirURL = xdg_user_dir_lookup("DOCUMENTS");

            return uno::makeAny( aDocumentsDirURL );
        }

        case SETTING_USER_GIVENNAME:
        {
            rtl::OUString aCompleteName( rtl::OStringToOUString(
                g_get_real_name(), osl_getThreadTextEncoding() ) );
            sal_Int32 nIndex = 0;
            rtl::OUString aGivenName;
            do
                aGivenName = aCompleteName.getToken( 0, ' ', nIndex );
            while ( nIndex == 0 );

            return uno::makeAny( aGivenName );

        }

        case SETTING_USER_SURNAME:
        {
            rtl::OUString aCompleteName( rtl::OStringToOUString(
                g_get_real_name(), osl_getThreadTextEncoding() ) );
            sal_Int32 nIndex = 0;
            rtl::OUString aSurname;
            do
                aSurname = aCompleteName.getToken( 0, ' ', nIndex );
            while ( nIndex >= 0 );

            return uno::makeAny( aSurname );
        }

        case SETTING_SOURCEVIEWFONT_NAME:
        case SETTING_SOURCEVIEWFONT_HEIGHT:
        {
            rtl::OUString aName;
            sal_Int16 nHeight;
                   
            splitFontName (aGconfValue, aName, nHeight);
            if (aValue.nSettingId == SETTING_SOURCEVIEWFONT_NAME)
                return uno::makeAny( aName );
            else
                return uno::makeAny( nHeight );
        }


        default:
            fprintf( stderr, "Unhandled setting to translate.\n" );
            break;
    }

    return uno::Any();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL isDependencySatisfied( GConfClient* aClient, const ConfigurationValue aValue )
{
    switch( aValue.nDependsOn )
    {
        case SETTING_PROXY_MODE:
        {
            GConfValue* aGconfValue = gconf_client_get( aClient, GCONF_PROXY_MODE_KEY, NULL );

            if ( aGconfValue != NULL ) 
            {
                bool bOk = g_strcasecmp( "manual", gconf_value_get_string( aGconfValue ) ) == 0;
                gconf_value_free( aGconfValue );
                if (bOk) return sal_True;
            }
        }
            break;

        case SETTING_WORK_DIRECTORY:
        {
            rtl::OUString aDocumentsDirURL = xdg_user_dir_lookup("DOCUMENTS");
            osl::Directory aDocumentsDir( aDocumentsDirURL );

            if( osl::FileBase::E_None == aDocumentsDir.open() )
                return sal_True;
        }
            break;

        case SETTING_USER_GIVENNAME:
        {
            rtl::OUString aCompleteName( rtl::OStringToOUString(
                g_get_real_name(), osl_getThreadTextEncoding() ) );
            if( !aCompleteName.equalsAscii( "Unknown" ) )
                return sal_True;
        }
            break;

        case SETTING_USER_SURNAME:
        {
            rtl::OUString aCompleteName( rtl::OStringToOUString(
                g_get_real_name(), osl_getThreadTextEncoding() ) );
            if( !aCompleteName.equalsAscii( "Unknown" ) )
            {
                if( aCompleteName.trim().indexOf(rtl::OUString::createFromAscii(" "), 0) != -1 )
                    return sal_True;
            }
        }
            break;

#ifdef ENABLE_LOCKDOWN
        case SETTING_AUTO_SAVE:
        {
            GConfValue* aGconfValue = gconf_client_get( aClient, GCONF_AUTO_SAVE_KEY, NULL );

            if( ( aGconfValue != NULL ) )
            {
                bool bOk = gconf_value_get_bool( aGconfValue );
                gconf_value_free( aGconfValue );
                if (bOk) return sal_True;
            }
        }
            break;
#endif // ENABLE_LOCKDOWN

        default:
            fprintf( stderr, "Unhandled setting to check dependency.\n" );
            break;
    }

    return sal_False;
}

}

ConfigurationValue const ConfigurationValues[] =
{
    {
        SETTING_ENABLE_ACCESSIBILITY,
        "/desktop/gnome/interface/accessibility",
        "EnableATToolSupport",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_PROXY_MODE,
        GCONF_PROXY_MODE_KEY,
        "ooInetProxyType",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_PROXY_HTTP_HOST,
        "/system/http_proxy/host",
        "ooInetHTTPProxyName",
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_HTTP_PORT,
        "/system/http_proxy/port",
        "ooInetHTTPProxyPort",
        sal_False,
        SETTING_PROXY_MODE
    },

	 {
        SETTING_PROXY_HTTPS_HOST,
        "/system/proxy/secure_host",
        "ooInetHTTPSProxyName",
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_HTTPS_PORT,
        "/system/proxy/secure_port",
        "ooInetHTTPSProxyPort",
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_FTP_HOST,
        "/system/proxy/ftp_host",
        "ooInetFTPProxyName",
        sal_False,
        SETTING_PROXY_MODE
    },

    {
        SETTING_PROXY_FTP_PORT,
        "/system/proxy/ftp_port",
        "ooInetFTPProxyPort",
        sal_False,
        SETTING_PROXY_MODE
    },
    
    {
        SETTING_NO_PROXY_FOR,
        "/system/http_proxy/ignore_hosts",
        "ooInetNoProxy",
        sal_True,
        SETTING_PROXY_MODE
    },

    {
        SETTING_MAILER_PROGRAM,
        "/desktop/gnome/url-handlers/mailto/command",
        "ExternalMailer",
        sal_True,
        SETTINGS_LAST
    },
    {
        SETTING_SOURCEVIEWFONT_NAME,
        "/desktop/gnome/interface/monospace_font_name",
        "SourceViewFontName",
        sal_True,
        SETTINGS_LAST
    },
    {
        SETTING_SOURCEVIEWFONT_HEIGHT,
        "/desktop/gnome/interface/monospace_font_name",
        "SourceViewFontHeight",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_WORK_DIRECTORY,
        "/desktop/gnome/url-handlers/mailto/command", // dummy
        "WorkPathVariable",
        sal_True,
        SETTING_WORK_DIRECTORY, // so that the existence of the dir can be checked
	},

#ifdef ENABLE_LOCKDOWN
    {
        SETTING_WRITER_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/writer_default_document_format",
        "TextDocumentSetupFactoryDefaultFilter",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_IMPRESS_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/impress_default_document_format",
        "PresentationDocumentSetupFactoryDefaultFilter",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_CALC_DEFAULT_DOC_FORMAT,
        "/apps/openoffice/calc_default_document_format",
        "SpreadsheetDocumentSetupFactoryDefaultFilter",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_AUTO_SAVE,
        GCONF_AUTO_SAVE_KEY,
        "AutoSaveEnabled",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_AUTO_SAVE_INTERVAL,
        "/apps/openoffice/auto_save_interval",
        "AutoSaveTimeIntervall",
        sal_False,
        SETTING_AUTO_SAVE
    },

    {
        SETTING_USER_GIVENNAME,
        "/desktop/gnome/url-handlers/mailto/command", // dummy
        "givenname",
        sal_True,
        SETTING_USER_GIVENNAME
    },

    {
        SETTING_USER_SURNAME,
        "/desktop/gnome/url-handlers/mailto/command", // dummy
        "sn",
        sal_True,
        SETTING_USER_SURNAME
    },

    {
        SETTING_DISABLE_PRINTING,
        "/desktop/gnome/lockdown/disable_printing",
        "DisablePrinting",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_USE_SYSTEM_FILE_DIALOG,
        "/apps/openoffice/use_system_file_dialog",
        "UseSystemFileDialog",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_PRINTING_MODIFIES_DOCUMENT,
        "/apps/openoffice/printing_modifies_doc",
        "PrintingModifiesDocument",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_ICONS_IN_MENUS,
        "/apps/openoffice/show_menu_icons",
        "ShowIconsInMenues",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_INACTIVE_MENUITEMS,
        "/apps/openoffice/show_menu_inactive_items",
        "DontHideDisabledEntry",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_FONT_PREVIEW,
        "/apps/openoffice/show_font_preview",
        "ShowFontBoxWYSIWYG",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SHOW_FONT_HISTORY,
        "/apps/openoffice/show_font_history",
        "FontViewHistory",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_ENABLE_OPENGL,
        "/apps/openoffice/use_opengl",
        "OpenGL",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_OPTIMIZE_OPENGL,
        "/apps/openoffice/optimize_opengl",
        "OpenGL_Faster",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_USE_SYSTEM_FONT,
        "/apps/openoffice/use_system_font",
        "AccessibilityIsSystemFont",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_USE_FONT_ANTI_ALIASING,
        "/apps/openoffice/use_font_anti_aliasing",
        "FontAntiAliasingEnabled",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_FONT_ANTI_ALIASING_MIN_PIXEL,
        "/apps/openoffice/font_anti_aliasing_min_pixel",
        "FontAntiAliasingMinPixelHeight",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_CREATE_PDF,
        "/apps/openoffice/lockdown/warn_info_create_pdf",
        "WarnCreatePDF",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_PRINT_DOC,
        "/apps/openoffice/lockdown/warn_info_printing",
        "WarnPrintDoc",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_SAVEORSEND_DOC,
        "/apps/openoffice/lockdown/warn_info_saving",
        "WarnSaveOrSendDoc",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_SIGN_DOC,
        "/apps/openoffice/lockdown/warn_info_signing",
        "WarnSignDoc",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_REMOVE_PERSONAL_INFO,
        "/apps/openoffice/lockdown/remove_personal_info_on_save",
        "Scripting/RemovePersonalInfoOnSaving",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_RECOMMEND_PASSWORD,
        "/apps/openoffice/lockdown/recommend_password_on_save",
        "RecommendPasswordProtection",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_UNDO_STEPS,
        "/apps/openoffice/undo_steps",
        "UndoSteps",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_SYMBOL_SET,
        "/apps/openoffice/icon_size",
        "SymbolSet",
        sal_True,
        SETTINGS_LAST
    },

    {
        SETTING_MACRO_SECURITY_LEVEL,
        "/apps/openoffice/lockdown/macro_security_level",
        "MacroSecurityLevel",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_CREATE_BACKUP,
        "/apps/openoffice/create_backup",
        "CreateBackup",
        sal_False,
        SETTINGS_LAST
    },

    {
        SETTING_WARN_ALIEN_FORMAT,
        "/apps/openoffice/warn_alien_format",
        "WarnAlienFormat",
        sal_False,
        SETTINGS_LAST
    },

#endif // ENABLE_LOCKDOWN
};

std::size_t const nConfigurationValues =
    sizeof ConfigurationValues / sizeof ConfigurationValues[0];

css::beans::Optional< css::uno::Any > getValue(ConfigurationValue const & data)
{
    GConfClient* aClient = getGconfClient();
    GConfValue* aGconfValue;
    if( ( data.nDependsOn == SETTINGS_LAST ) || isDependencySatisfied( aClient, data ) )
    {
        aGconfValue = gconf_client_get( aClient, data.GconfItem, NULL );

        if( aGconfValue != NULL )
        {
            css::uno::Any value;
            if( data.bNeedsTranslation )
                value = translateToOOo( data, aGconfValue );
            else
                value = makeAnyOfGconfValue( aGconfValue );

            gconf_value_free( aGconfValue );

            return css::beans::Optional< css::uno::Any >(true, value);
        }
    }
    return css::beans::Optional< css::uno::Any >();
}

}
