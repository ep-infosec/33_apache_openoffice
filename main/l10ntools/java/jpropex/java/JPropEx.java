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



 
import java.util.*;
import java.io.*;

public class JPropEx
{
    private String inputFileArg     = "";
    private String outputFileArg    = "";
    private String pathPrefixArg    = "";
    private String pathPostfixArg   = "";
    private String projectArg       = "";
    private String rootArg          = "";
    private Vector forcedLangsArg;
    private Vector langsArg;
    private String inputSdfFileArg  = "";
    private boolean isQuiet             = false;
    private final String resourceType   = "javaproperties";
    private final String sourceLanguage = "en-US";
    
    static final int JAVA_TYPE      = 0;
    static final int JAVA_ENUS_TYPE = 1;
    static final int EXTENSION_TYPE = 2;

    public JPropEx()
    {
        //data = new SdfData();
    }
    
    public JPropEx( String args[] )
    {
        super();
        parseArguments( args );
        testCL();
        //testArguments();
        if( inputSdfFileArg != null && inputSdfFileArg.length() > 0 ) 
            merge(); 
        else
            extract();
    } 

    private String getSimpleArg( String[] args , int x )
    {
        if( x < args.length ) x++;
        else 
        {
            System.err.println("ERROR: Missing arg for "+args[ x ]+"\n");
            help();
        }
        return args[ x ];
    }
    private Vector getComplexArg( String[] args , int x )
    {
        if( x < args.length ) x++;
        else
        {
            System.err.println("ERROR: Missing arg for "+args[ x ]+"\n");
            help();
        }
        String value = args[ x ];
        Vector values = new Vector( Arrays.asList( value.split(",") ) );
        return values;
    }

    private void testCL()
    {
        if( inputFileArg.length()>0 && ( ( pathPrefixArg.length()>0 && pathPostfixArg.length()>0 ) || outputFileArg.length()>0 ) && projectArg.length()>0 && rootArg.length()>0 && langsArg.size()>0 )
            if( ( inputSdfFileArg.length()>0 && ( outputFileArg.length()>0 ||  ( pathPrefixArg.length()>0 && pathPostfixArg.length()>0 ) ) ) ||  ( inputFileArg.length()>0 && outputFileArg.length()>0 ) )  
                    return;
        System.out.println("ERROR: Strange parameters!");
        help();
        System.exit( -1 );
    }
    private void help()
    {
        System.out.println("jpropex -> extract / merge java properties files");
        System.out.println("-p <project> -r <project_root> -i <input>|@<input> -o <output> -x <path1> -y <path2> -m <sdf_file> -l <lang>\n");
        System.out.println("Example:\ncd /data/cws/l10ntooling17/DEV300/ooo/reportbuilder/java/com/sun/star/report/function/metadata");
        System.out.println("Extract:\njpropex -p reportbuilder -r ../../../../../../.. -i Title-Function.properties -o new.sdf -l en-US");
        System.out.println("Merge: use either ( -x path -y more_path ) or ( -o ) and ( -i filename ) or ( -i @filename ). @filename contains a list with files");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivo -i @abc -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivo -i @abc -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");        
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -o ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata/ -i Title-Function.properties -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.out.println("jpropex -p reportbuilder -r ../../../../../../.. -x ../../../../../../../unxlngx6.pro/class/com/sun/star/report/function/metadata -y ivooo -i Title-Function.properties -l all -lf en-US,de,fr,pt -m ../../../../../../../common.pro/misc/reportbuilder/java/com/sun/star/report/function/metadata/localize.sdf");
        System.exit( -1 );
    }

    private void extract()
    {
        SdfData data = new SdfData();
        java.util.Properties prop = loadProp( inputFileArg );
       
        // Get a prototype that already contains the most common settings
        SdfEntity dolly = prepareSdfObj( inputFileArg );
        String key;
        SdfEntity currentStr;
        String value;
        String str;
        for( Enumeration e = prop.propertyNames() ; e.hasMoreElements() ; )
        {
            key         = (String)      e.nextElement();
            currentStr  = (SdfEntity)   dolly.clone();
            // Set the new GID and the string text
            currentStr.setGid( key );
            value            = prop.getProperty( key , "" ); 
            //if( value.equals("") )  System.err.println("Warning: in file "+inputFileArg+" the string with the key "+key+" has a empty string!");
            str = (prop.getProperty( key )).replaceAll("\t" , " " );    // remove tab
            str = str.replaceAll("\n"," ");                             // remove return
            currentStr.setText( str );     
            if( str.length() > 0 )
                data.add( currentStr );
        }
        data.write( outputFileArg );
    }
    
    private SdfEntity prepareSdfObj( String filename )
    {
        String path = makeAbs( filename ).trim();
        String myRootArg = makeAbs( rootArg ).trim();
        myRootArg = myRootArg.replace( "\\","/");
        myRootArg += "/";
        path = path.replace("\\","/");
        path = path.replace( myRootArg, "" );
        path = path.replace("/","\\");
        // TODO: Make this static
        java.text.SimpleDateFormat dateformat = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String date = dateformat.format( new Date() );
        return new SdfEntity( projectArg , path , "0" /* dummy1 */ , resourceType , "", "" , "" , "" , "0" /* dummy2 */ , 
                              sourceLanguage , "",  "" , ""  , "" , date );
    }
    
    private void merge()
    {
        SdfData data = getSdfData();
        if( inputFileArg.startsWith("@") )
        {
            // Read files
            Vector fileList = readFileList( inputFileArg );
            for( Enumeration e = fileList.elements(); e.hasMoreElements(); )
                mergeFile( (String) e.nextElement() , data , false );
        }
        else
        {
            // Single file
            mergeFile( inputFileArg , data , true );
        }
    }
    
    private Vector readFileList( String filename )
    {
        Vector lines = new Vector();
        try
        {
            BufferedReader in = new BufferedReader( new FileReader( filename.substring( 1 ) ) );
            while( in.ready() )
                lines.add( in.readLine().trim() );
        }
        catch( IOException e )
        {
            System.out.println("ERROR: Can't open file '"+filename.substring( 1 )+"'");
            System.exit( -1 );
        }
        return lines; 
    }
   
    private void mergeFile( String filename , SdfData data , boolean isSingleFile )
    {
        int type = detectFormat( filename );
        java.util.Properties sourceProp = loadProp( filename );
        Vector langs = getLanguages( data );
        HashMap props = new HashMap();
        // Create a properties object for every language
        for( Enumeration e = langs.elements(); e.hasMoreElements();)
        {
            props.put( (String)e.nextElement() , new java.util.Properties() );
        }
        // Get a prototype that already contains the most common settings
        
        SdfEntity dolly = prepareSdfObj( filename );
        String key;
        String sourceString;
        SdfEntity curStr;
        SdfEntity curEntity;
        SdfEntity mergedEntity;
        String curLang;
        for( Enumeration e = sourceProp.propertyNames() ; e.hasMoreElements() ; )     // For all property keys
        {
            key          = (String) e.nextElement();
            sourceString = sourceProp.getProperty( key );
            curStr       = (SdfEntity) dolly.clone();
            curStr.setGid( key );
            for( Enumeration lang = langs.elements(); lang.hasMoreElements(); ) // merge in every language
            {
                curEntity   = (SdfEntity) curStr.clone();
                curLang     = (String)    lang.nextElement();
                curEntity.setLangid( curLang );
                mergedEntity = data.get( curEntity );
                if( mergedEntity == null )
                {
                    // in case there is no translation then fallback to the en-US source string
                    ( (java.util.Properties) props.get( curLang )).setProperty( curEntity.getGid() , sourceString  );
                }
                else
                {
                    // Set the merged text from the sdf file
                    ( (java.util.Properties) props.get( curLang )).setProperty( mergedEntity.getGid() , mergedEntity.getText() );  // TODO: Quoting ???
                }
            }

        }
        // Now write them out
        String lang;
        for( Iterator i = props.keySet().iterator() ; i.hasNext() ; )
        {
            lang = (String) i.next();
            writeSinglePropertiesFile( filename , (java.util.Properties) props.get( lang ) , lang , isSingleFile , type );
        }
    }
    private void writeSinglePropertiesFile( String filename , java.util.Properties prop , String lang , boolean isSingleFile , int type )
    {
        // Prepare path to file
        int filenameIdx     = filename.lastIndexOf( "/" ) > 0 ? filename.lastIndexOf( "/" )+1 : 0 ;
        String path         = new String();
        String name         = new String();
        String lcLang       = lang.toLowerCase();
        // use of -x <path> -y <more_path>
        // -> <path>/<lang>/<more_path>
        if( pathPrefixArg != null && pathPrefixArg.length()>0 && pathPostfixArg != null && pathPostfixArg.length()>0 )
        {    
            path = new StringBuffer().append( pathPrefixArg ).append( "/" ).append( lcLang ).append( "/" ).append( pathPostfixArg ).append( "/" ).toString();
            name += formatFilename( filename , filenameIdx , lang , type );
        }
        //use of -i <one_filename>
        else if( !isSingleFile && outputFileArg != null && outputFileArg.length()>0 )
        {
            //name = outputFileArg;
            path = outputFileArg;
            name += formatFilename( filename , filenameIdx , lang , type );
        }
        //use of -i @<file_containing_many_filenames>
        else if( isSingleFile && outputFileArg != null && outputFileArg.length()>0 )
        {
            //name = outputFileArg;
            path = outputFileArg;
            name += formatFilename( filename , filenameIdx , lang , type );
        }
        else
        {
            System.err.println("ERROR: No outputfile specified .. either -o or -x -y !");
            System.exit( -1 );
        }

        File dir = new File( path );
        try
        {
            if( !dir.exists() && path.length()>0 )
            {
                if( !dir.mkdirs() )
                {
                    System.out.println("ERROR: Can't create directory '"+path+"' !!!");
                    System.exit( -1 );
                }
            }
        }
        catch( SecurityException e )
        {
            System.out.println("ERROR: Can't create directory '"+path+"'!!!Wrong Permissions?");
            System.exit( -1 );
        }
        path += name;
        // Write the properties file
        //System.out.println("DBG: Writing to "+path);
        try{
            BufferedOutputStream out = new BufferedOutputStream( new FileOutputStream( path ) );
            if( prop == null )
                System.out.println("DBG: prop == null!!!");
            prop.store( out , "" );     // Legal headers?
        }
        catch( IOException e )
        {
            System.out.println("ERROR: Can't write file '"+path+"' !!!!");
            System.exit( -1 );
        }
    }
    
    // we have different types of properties in the source code
    // each needs a different file nameing scheme
    private int detectFormat( String filename )
    {
       if( filename.endsWith( "_en_US.properties" ) )
           return EXTENSION_TYPE;
       else if( filename.endsWith("_en_us.properties" ) )
           return JAVA_ENUS_TYPE;
       else if( filename.endsWith( ".properties" ) )
           return JAVA_TYPE;
       
       // Can not detect, exit
       System.err.println("ERROR: Invalid file name. Only allowed (case sensitive!)  *_en_US.properties , *_en_us.properties or *.properties\n");
       System.exit(-1);
       return JAVA_TYPE;    // dummy
    }
    
    private String formatFilename( String filename , int filenameIdx , String lang , int type )
    {
       
        if( !lang.equals( "en-US" ) )
        {
            // Parse iso code
            int pos = lang.indexOf("-");
            String langpart1 = new String();
            String langpart2 = new String();
            if( pos == -1 )
            {
                langpart1 = lang;
            }
            else if( pos > 0 )
            {
                langpart1 = lang.substring( 0 , pos );
                langpart2 = lang.substring( pos+1 , lang.length() );
            }
            // change filename according to the type
            switch( type )
            {
                // -> de_DE
                case EXTENSION_TYPE: 
                    lang  =  langpart1.toLowerCase(); 
                    if( langpart2.length() > 0 )                    // -> en_US
                        lang += "_" + langpart2.toUpperCase();
                    else                                            // -> de_DE
                        lang += "_" + langpart1.toUpperCase();
                    return new StringBuffer().append( filename.substring( filenameIdx , filename.lastIndexOf( "_en_US.properties" ) ) )
                                        .append( "_" ).append( lang.replaceAll("-","_") ).append( ".properties" ).toString();
                    // -> de
                case JAVA_ENUS_TYPE:
                    lang = langpart1.toLowerCase();
                    if( langpart2.length() > 0 )
                        lang += "_" + langpart2.toLowerCase();
                    return new StringBuffer().append( filename.substring( filenameIdx , filename.lastIndexOf( "_en_us.properties" ) ) )
                                        .append( "_" ).append( lang.replaceAll("-","_") ).append( ".properties" ).toString();
                    // -> de
                case JAVA_TYPE:
                    lang = langpart1.toLowerCase();
                    if( langpart2.length() > 0 )
                        lang += "_" + langpart2.toLowerCase();
                    return new StringBuffer().append( filename.substring( filenameIdx , filename.lastIndexOf( ".properties" ) ) )
                                        .append( "_" ).append( lang.replaceAll("-","_") ).append( ".properties" ).toString();
                default:
                        System.err.println("ERROR: Something is really broken here, l10ntools/java/jprop/java/JPropEx.java :: formatFilename()");
                        System.exit( -1 );
                break;
            }
       }
        return filename;        // don't change en-US source file name
    }
    private SdfData getSdfData()
    {
        SdfData data = new SdfData( inputSdfFileArg );
        data.read();
        return data;
    }
    private Vector getLanguages( SdfData data )
    {
        Vector langs = new Vector();

        if( ((String)langsArg.get( 0 )).equalsIgnoreCase( "all" ) ) // for "-l all" use all languages found in the -m sdf file 
            langs.addAll( data.getLanguages() );
        else
            langs.addAll( langsArg );              // use the langs giving by -l

        if( forcedLangsArg != null ) 
            langs.addAll( forcedLangsArg );
        
        return removeDupes( langs );
    } 
    private Vector removeDupes( Vector vec )
    {
        Collection coll = new LinkedHashSet( vec );
        return new Vector( coll );
    }
    private java.util.Properties loadProp( String filename )
    {
        java.util.Properties prop = new java.util.Properties();
        try
        {
            prop.load( new BufferedInputStream( new NoLocalizeFilter( new FileInputStream( filename ) ) ) );
        }
        catch( IOException e )
        {
            System.err.println("ERROR: Can't read file '"+filename+"'!!!");
        }
        return prop;
    }
    private void parseArguments( String[] args )
    {
        
        if( args.length == 0 )
        {
            System.out.println("ERROR: No args???");
            help();
            System.exit( -1 );
        }
        for( int x = 0; x < args.length ; x++ )
        {
            if( args[ x ].equalsIgnoreCase("-i") ) 
		    {
                // Input resource file
                inputFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-o") )
            {
                // Output sdf file
                outputFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-x") )
            {
                // path prefix
                pathPrefixArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-y") )
            {
                // path postfix
                pathPostfixArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-p") )
            {
                // project
                projectArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-r") )
            {
                // root
                rootArg = getSimpleArg( args , x );
                rootArg = makeAbs( rootArg );
            }
            else if( args[ x ].equalsIgnoreCase("-lf") )
            {
                // forced langs
                forcedLangsArg = getComplexArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-l") )
            {
                // langs
                langsArg = getComplexArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-m") )
            {
                // input sdf file
                inputSdfFileArg = getSimpleArg( args , x );
            }
            else if( args[ x ].equalsIgnoreCase("-qq") )
            {
                isQuiet = true; 
            }
        }
    }
    private String makeAbs( String path )
    {
        File file;
        try
        {    
            file = new File( path );
            return file.getCanonicalPath();
        }catch( IOException e )
        {
            e.printStackTrace();
            System.exit( -1 );
        }
        return null;
    }
/*    private boolean testArguments()
    {
        // nice merge 
        if( inputSdfFileArg != null && inputSdfFileArg.length()>0 ) 
            // nice merge
            return  projectArg != null  && rootArg != null && inputFileArg != null && pathPrefixArg != null && pathPostfixArg != null && langsArg != null &&
                    projectArg.length()>0 && rootArg.length()>0 && inputFileArg.length()>0 && pathPrefixArg.length()>0 && pathPostfixArg.length()>0 && langsArg.size()>0 ;
        else 
            // nice extract
            return  projectArg != null && rootArg != null && inputFileArg != null && outputFileArg != null && langsArg != null &&
                    projectArg.length()>0 && rootArg.length()>0 && inputFileArg.length()>0 && outputFileArg.length()>0 && langsArg.size()>0;
    }
*/
}
