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


/*
 * Created on 2005
 *	by Christian Schmidt
 */
package com.sun.star.tooling.DirtyTags;

import java.io.IOException;
import java.util.Map;

import com.sun.star.tooling.converter.ExtMap;

/**
 * @author Christian Schmidt 2005
 *
 */
public class Tag {
    private static int indent=0;
    Map tagNames;
    private String tagType;
    private String tagName;
    private String tagString;
    public static Tag EMPTYTAG=new Tag("","","");

    /**
     * Create a new Instance of Tag
     * 
     * @param tagType
     * @param tagName
     * @param tagString
     */
    public Tag(String tagType, String tagName, String tagString) {
        
        this.tagType=tagType;
        this.tagName=tagName;
        this.tagString=tagString;
        
        tagNames=new ExtMap();
        tagNames.put("link","name");
        tagNames.put("caption","xml-lang");
        tagNames.put("alt","xml-lang");
    }
    
    public String getWrappedTagString() throws IOException{
        if(this.canHaveTranslateableContent()){
            return this.wrapTagStringIntern();
        }else{
            return xmlString(this.tagString);
        }
    }
    
    private final String xmlString( final String string) throws java.io.IOException {
        if (string == null)
            return string; // ""
        String str = string;

         for(int i=0;i<str.length();i++){
             if(str.charAt(i)=='&'){
                 str=str.substring(0, i)+"&amp;"+str.substring(i+1);
                 continue;
             }
    
             if(str.charAt(i)=='<'){
                 str=str.substring(0, i)+"&lt;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='>'){
                 str=str.substring(0, i)+"&gt;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='"'){
                 str=str.substring(0, i)+"&quot;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='\''){
                 str=str.substring(0, i)+"&apos;"+str.substring(i+1);
                 continue;
             }
         }

        return str;
    }
    /**
     * @return
     */
    private boolean canHaveTranslateableContent() {
        return (tagNames.containsKey(this.tagName));
        
    }

    /**
     * @throws IOException
     * 
     */
    private String wrapTagStringIntern() throws IOException {
        
    
        String[] split=this.tagString.split("=");
        int length=split.length;
        // no attribute found;
        if (length==0) return xmlString(tagString);
        else{
            int i=0;
            
            while(i<length-1/*the last part can only contain an attribute value*/){
                String attributeName = split[i].trim();
                if(split[i]. indexOf("</sub>")<0) split[i]=xmlString(split[i]);
                i++;
                String value;
                attributeName=(attributeName.substring(attributeName.lastIndexOf(" ")).trim());
                if((value=translateableAttributeValue(this.tagName)).equals(attributeName)){
                    int valueStart=0;
                    int valueEnd=0;
                    
                    // get the value to the found attribute name
                    // it must either be surrounded by '"'...
                    if((valueStart=split[i].indexOf('"'))>=0){
                        valueEnd = split[i].lastIndexOf('"');
                    //...or surrounded by "'"
                    }else if((valueStart=split[i].indexOf("'"))>=0){
                        valueEnd = split[i].lastIndexOf("'");
                    }else{
                        // there seems to be an error,
                        // we found an '=' (we split there) but no '"' or '''
                        // but although we don't check the syntax
                        // we just continue
                        continue;
                    }
                    //ok we found the border of a value that might be translated
                    //now we wrap it with the tags
                    
                    split[i]=xmlString(split[i].substring(0,valueStart+1))+"<sub>"+xmlString(split[i].substring(valueStart+1,valueEnd))+"</sub>"+xmlString(split[i].substring(valueEnd));

                }
            }
            String wrappedString="";
            // we have the wrapped parts, now we put them together
            int j=0;
            for(j=0;j<split.length-1;j++){
                wrappedString+=(split[j]+"=");   
            }
            wrappedString+=split[j];
//            System.out.println(this.tagString);
//            System.out.println(wrappedString);
            return wrappedString;
        }
        
    }



    /**
     * @param tagName the name of the tag to check
     * @return the name of the attribute  that can contain translateable value
     */
    private String translateableAttributeValue(String tagName) {

        return (String)this.tagNames.get(tagName);
    }


    /**
     * Create a new Instance of Tag
     * 
     * 
     */
    public Tag(String tagString) {
        this(extractTagType(extractTagName(tagString)),extractTagName(tagString),tagString);
    }
    
    private static String extractTagName(String tagString){
        
        int start=tagString.indexOf('<')+1;
        int end=tagString.lastIndexOf('\\');
        if(start>=0&&end>0){
            tagString=tagString.substring(start,end);
            
            if(tagString.indexOf(" ")>0){
               tagString=tagString.substring(0,tagString.indexOf(" "));
            }
            return tagString;
        }else{
            return "";
        }
    }
    private static String extractTagType(String tagName){
        if(tagName.equals("")){
            return "Text";
        }else if(tagName.startsWith("/")){
            return "EndTag";
        }else if(tagName.endsWith("/")){
            return "StartAndEndTag";
        }else {
            return "StartTag";
        }
    }

    /**
     * @return Returns the tagName.
     */
    public String getTagName() {
        return this.tagName;
    }
    /**
     * @return Returns the tagString.
     */
    public String getTagString() {
        return this.tagString;
    }
    /**
     * @return Returns the tagType.
     */
    public String getTagType() {
        return this.tagType;
    }
    

}
