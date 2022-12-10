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



#ifndef OOX_OLE_VBAINPUTSTREAM_HXX
#define OOX_OLE_VBAINPUTSTREAM_HXX

#include <vector>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {
namespace ole {

// ============================================================================

/** A non-seekable input stream that implements run-length decompression. */
class VbaInputStream : public BinaryInputStream
{
public:
    explicit            VbaInputStream( BinaryInputStream& rInStrm );

    /** Returns -1, stream size is not determinable. */
    virtual sal_Int64   size() const;
    /** Returns -1, stream position is not tracked. */
    virtual sal_Int64   tell() const;
    /** Does nothing, stream is not seekable. */
    virtual void        seek( sal_Int64 nPos );
    /** Closes the input stream but not the wrapped stream. */
    virtual void        close();

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 );
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 );

private:
    /** If no data left in chunk buffer, reads the next chunk from stream. */
    bool                updateChunk();

private:
    typedef ::std::vector< sal_uInt8 > ChunkBuffer;

    BinaryInputStream*  mpInStrm;
    ChunkBuffer         maChunk;
    size_t              mnChunkPos;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
