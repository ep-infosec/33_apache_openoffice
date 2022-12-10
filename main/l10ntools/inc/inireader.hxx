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

#include <string>
#include <hash_map>
#include <unicode/regex.h>

using namespace std;

namespace transex3
{

struct eqstr
{
    bool operator()( const string s1 , const string s2) const
    {
        return s1.compare( s2 ) == 0;
    }
};

typedef std::hash_map< string , string > stringmap;
typedef std::hash_map< string, stringmap* > INImap;

class INIreader
{
    private:
        UErrorCode section_status;  
        UErrorCode parameter_status; 
        RegexMatcher* section_match;
        RegexMatcher* parameter_match;

    public: 
        INIreader(): section_status   ( U_ZERO_ERROR ) ,
                     parameter_status ( U_ZERO_ERROR ) 
        {
                     section_match   = new RegexMatcher   ( "^\\s*\\[([a-zA-Z0-9]*)\\].*" , 0 , section_status );
                     parameter_match = new RegexMatcher   ( "^\\s*([a-zA-Z0-9]*)\\s*=\\s*([a-zA-Z0-9 ]*).*" , 0 , parameter_status ) ;
        }
        ~INIreader()
        {
            delete section_match;
            delete parameter_match;
        }
        // open "filename", fill hash_map with sections / paramaters 
        bool read( INImap& myMap , string& filename );
       
    private:
        bool is_section( string& line , string& section_str );
        bool is_parameter( string& line , string& parameter_key , string& parameter_value );
        inline void check_status( UErrorCode status );
        inline void toStlString ( const UnicodeString& str, string& stl_str );
        inline void trim( string& str );
};

}
