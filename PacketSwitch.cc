#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include "PacketSwitch.hh"
#include "mypacket.hh"
#include <click/packet_anno.hh>

CLICK_DECLS

PacketSwitch::PacketSwitch() {click_chatter("Switch constructor");}
PacketSwitch::~PacketSwitch(){}
int PacketSwitch::initialize(ErrorHandler *errh){
	click_chatter("Swicth Error handler");
    return 0;
}
int PacketSwitch::configure(Vector<String> &conf, ErrorHandler *errh) {
	click_chatter("Switch Return");
        return 0;
}
void PacketSwitch::push(int port, Packet *packet) {
	assert(packet); //packets are queued
        int receiver_port = static_cast<int>(packet->anno_u8(8));   	
        struct commonheader *header = (struct commonheader *)packet->data();
        if (header->type == DATA){
                 click_chatter("Data packet with seqnum %d at port %d",header->seq_num,receiver_port);
                         routerq[receiver_port].push(packet);
                         Packet *pkt = routerq[receiver_port].front()->clone();
                         checked_output_push(receiver_port, pkt);                 }
                 else 
   		 	checked_output_push(receiver_port, packet);  
}
CLICK_ENDDECLS
EXPORT_ELEMENT(PacketSwitch)