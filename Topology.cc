#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include <click/packet_anno.hh>

#include "Topology.hh"
#include "mypacket.hh"


CLICK_DECLS

Topology::Topology():port_timer(this) {
   //click_chatter("Topology constructor");
    	seqnum = 0;
    	port_period = 5;
  }
Topology::~Topology(){}
int Topology::initialize(ErrorHandler *errh){
//click_chatter("Topolofy initializer");
    	port_timer.initialize(this);
    	port_timer.schedule_after_sec(1);
    	return 0;
}

void Topology::build_portstable() {
	click_chatter("Topology portstable");
     	int headroom = sizeof(click_ether);
     	WritablePacket *packet = Packet::make(headroom,0,sizeof(struct hellopacket), 0);
     	memset(packet->data(),0,packet->length());
     	struct hellopacket *format = (struct hellopacket*) packet->data();
        format->header.type = HELLO;
        format->header.src_address = uint16_t(myaddress);
        format->header.seq_num = seqnum++;    
        typedef std::map<int,int>::const_iterator ii;
        for (ii iter = portstable.begin(); iter != portstable.end(); iter++)
        {
             	click_chatter("Sending hello from %d at port %d", iter->first, iter->second); 
               	packet->set_anno_u8(0, iter->second);
                output(0).push(packet); 
         }

}
void Topology::run_timer(Timer* timer){
         build_portstable();
         port_timer.schedule_after_sec(port_period);}
std::map<int,int>& Topology::get_port(){ //send ports table
  return portstable;}

int Topology::check_port(int destination){ //check for the presence of the port
               portstable_iterator temp_it = portstable.find(destination);
               if(temp_it != portstable.end()) {
                      return temp_it->second;  }
               else
                      return -1;}
int Topology::configure(Vector<String> &conf, ErrorHandler *errh) {
	click_chatter("Topo configure");
        if (cp_va_kparse(conf, this, errh,"MY_ADDRESS", cpkM, cpInteger, &myaddress, cpEnd) < 0) {
                  return -1;
        }
        return 0;}
int Topology::get_seqnum(){
	return seqnum++;}
void Topology::push(int port, Packet *packet) {
	assert(packet);
	struct commonheader *header = (struct commonheader *)packet->data();
        if (header->type == HELLO) { 
		click_chatter("Received Hello from %u on port %d", header->src_address, port); //if hello packet is received
                portstable[header->src_address] = port;
               	typedef std::map<int,int>::const_iterator ii;
        	click_chatter("-----Ports Table of Router %d-------", myaddress);
	for (ii iter = portstable.begin(); iter != portstable.end(); iter++)	{
    		click_chatter("Destination: %d ---- Port: %d", iter->first, iter->second);
		}
        click_chatter("--------------------------------------");
                int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ackpacket), 0);
                memset(ack->data(),0,ack->length());
                struct ackpacket *format = (struct ackpacket*) ack->data();
              	format->header.type = ACK;
               	format->header.seq_num = header->seq_num; 
               	format->header.src_address = uint16_t(myaddress);
               	format->dst_address = header->src_address;
		int src = static_cast<int> (header->src_address);
		packet->kill();
		int dest_port = check_port(src);
                if (dest_port == -1){
                   click_chatter( "Value not present in the ports table. KILL packet.\n");
                    packet->kill();  }
		else {
                    ack->set_anno_u8(8, dest_port);
                    output(0).push(ack); }
		  }
        else {
		click_chatter("Wrong packet type");
		packet->kill();	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Topology)
