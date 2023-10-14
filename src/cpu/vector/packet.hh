#ifndef __CPU_VECTOR_PACKET_HH__
#define __CPU_VECTOR_PACKET_HH__

#include <cstdint>

#include "mem/packet.hh"
#include "mem/request.hh"

class VectorPacket;

typedef VectorPacket * VectorPacketPtr;

class VectorPacket: public Packet
{
    VectorPacket(const RequestPtr &req, MemCmd cmd, uint64_t req_id, uint8_t channel)
    : Packet(req, cmd), reqId(req_id), channel(channel){}

    
}