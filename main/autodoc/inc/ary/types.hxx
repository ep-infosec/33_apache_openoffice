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



#ifndef ARY_TYPES_HXX
#define ARY_TYPES_HXX

// USED SERVICES
    // BASE CLASSES
    // OTHER


namespace ary
{


typedef uintt           Rid;
typedef uintt           ClassId;



// Deprecated:
typedef Rid             Gid;        /// Group Id. Id of a group.
typedef UINT8           SlotAccessId;   /// Access to a Slot
typedef std::set< Rid, std::less< Rid > >   Set_Rid;
typedef std::vector<Rid>	                List_Rid;





/** This is a global id, providing as well an entity's class as its
    id.
*/
class GlobalId
{
  public:
                        GlobalId()
                            :   nClass(0),
                                nId(0) {}
                        GlobalId(
                            ClassId             i_class,
                            Rid                 i_id )
                            :   nClass(i_class),
                                nId(i_id) {}
                        ~GlobalId() {}

    bool                IsValid() const         { return nClass != 0
                                                  AND
                                                  nId != 0; }
    ClassId             Class() const           { return nClass; }
    Rid                 Id() const              { return nId; }

  private:
    // DATA
    ClassId             nClass;
    Rid                 nId;
};


typedef std::vector<GlobalId>   List_GlobalIds;


/** This is a typed repository id. It allows to get
    an object of a specific type.
*/
template <class IFC>
class TypedId
{
  public:
    typedef TypedId<IFC>                        self;


    explicit            TypedId(
                            Rid                 i_nId = 0 )
                                                : nId(i_nId) {}
    TypedId<IFC> &      operator=(
                            Rid                 i_nId )
                                                { nId = i_nId; return *this; }
    bool                operator==(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return nId == i_nId.nId; }
    bool                operator!=(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return NOT operator==(i_nId); }
    bool                operator<(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return nId < i_nId.nId; }

    bool                IsValid() const         { return nId != 0; }
    Rid                 Value() const           { return nId; }

    static self         Null_()                 { return self(0); }

  private:
    // DATA
    Rid                 nId;
};




}   // namespace ary
#endif
