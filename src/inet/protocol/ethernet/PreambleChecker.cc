//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/protocol/ethernet/PreambleChecker.h"

namespace inet {

Define_Module(PreambleChecker);

void PreambleChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
    }
}

bool PreambleChecker::matchesPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<EthernetPhyHeader>();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    if (auto interfaceEntry = findContainingNicModule(this))
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    return header->getPreambleType() == SFD;
}

} // namespace inet
