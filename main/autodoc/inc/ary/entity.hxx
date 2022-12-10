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



#ifndef ARY_ENTITY_HXX
#define ARY_ENTITY_HXX

// BASE CLASSES
#include <ary/object.hxx>




namespace ary
{


/** Interface for every class, whose objects are searchable within the
    Autodoc Repository by an id.

    @todo
    Possibly make ->Set_Id() less public accessible.
*/
class Entity : public Object
{
  public:
    virtual             ~Entity() {}

    Rid                 Id() const;

    /// @attention Must be used only by ->ary::stg::Storage<>
    void                Set_Id(
                            Rid                 i_nId );
  protected:
                        Entity()      : nId(0) {}
  private:
    // DATA
    Rid                 nId;
};


inline Rid
Entity::Id() const
{
    return nId;
}

inline void
Entity::Set_Id(Rid i_nId)
{
    nId = i_nId;
}




}   // namespace ary
#endif
