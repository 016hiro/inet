//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPLWIP_DATASTREAMQUEUES_H
#define __INET_TCPLWIP_DATASTREAMQUEUES_H

#include <omnetpp.h>

#include "TcpLwipQueues.h"

#include "ByteArrayBuffer.h"
#include "TCPConnection.h"

/**
 * Send/Receive queue that manages "data stream", that is, valid bytes.
 */

class INET_API TcpLwipDataStreamSendQueue : public TcpLwipSendQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipDataStreamSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipDataStreamSendQueue();

    virtual void setConnection(TcpLwipConnection *connP);

    virtual void enqueueAppData(cPacket *msgP);

    virtual unsigned int getBytesForTcpLayer(void* bufferP, unsigned int bufferLengthP) const;

    virtual void dequeueTcpLayerMsg(unsigned int msgLengthP);

    unsigned long getBytesAvailable() const;

    virtual TCPSegment* createSegmentWithBytes(const void *tcpDataP, unsigned int tcpLengthP);

    virtual void discardAckedBytes(unsigned long bytesP);

  protected:
    ByteArrayBuffer byteArrayBufferM;
};

class INET_API TcpLwipDataStreamReceiveQueue : public TcpLwipReceiveQueue
{
  public:
    /**
     * Ctor.
     */
    TcpLwipDataStreamReceiveQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TcpLwipDataStreamReceiveQueue();

    // see TcpLwipReceiveQueue
    virtual void setConnection(TcpLwipConnection *connP);

    // see TcpLwipReceiveQueue
    virtual void notifyAboutIncomingSegmentProcessing(TCPSegment *tcpsegP, uint32 seqNo, const void* bufferP, size_t bufferLengthP);

    // see TcpLwipReceiveQueue
    virtual void enqueueTcpLayerData(void* dataP, unsigned int dataLengthP);

    // see TcpLwipReceiveQueue
    virtual unsigned long getExtractableBytesUpTo() const;

    // see TcpLwipReceiveQueue
    virtual TCPDataMsg* extractBytesUpTo(unsigned long maxBytesP);

    // see TcpLwipReceiveQueue
    virtual uint32 getAmountOfBufferedBytes() const;

    // see TcpLwipReceiveQueue
    virtual uint32 getQueueLength() const;

    // see TcpLwipReceiveQueue
    virtual void getQueueStatus() const;

    // see TcpLwipReceiveQueue
    virtual void notifyAboutSending(const TCPSegment *tcpsegP);

  protected:
    /// store bytes
    ByteArrayBuffer byteArrayBufferM;
};

#endif // __INET_TCPLWIP_DATASTREAMQUEUES_H
