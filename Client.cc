#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/string.hh>
#include <click/packet_anno.hh>
#include <string.h>
#include "Client.hh"
#include "mypacket.hh"

CLICK_DECLS

Client::Client(): ports_timer(this), TimerTo_timer(this){
	click_chatter("Client: Constructor initialized");
	//click_chatter("seqnum");
        seqnum = 0;
	//click_chatter("delay");
        delay = 0;
	//click_chatter("retransmit");
        retransmissions = 0;
	//click_chatter("timeout");
        time_out = 1;
	//click_chatter("k");
        k = 0;
	//click_chatter("dst1");
        dst1 = 0;
	//click_chatter("dst2");
        dst2 = 0;
	//click_chatter("dst3");
        dst3 = 0;
	//click_chatter("address");
        myaddress = 0;
	//click_chatter("DONE");
 }
Client::~Client(){}
int Client::configure(Vector<String> &conf, ErrorHandler *errh) {  
	click_chatter("Initialize configure");
   	if (cp_va_kparse(conf, this, errh, "PAYLOAD", cpkP+cpkM, cpString, &payload,"K", cpkM, cpInteger, &k, "MY_ADDRESS", cpkM, cpInteger, &myaddress,"DST1", cpkM, cpInteger, &dst1,"DST2", cpkM, cpInteger, &dst2,"DST3", cpkM, cpInteger, &dst3,"DELAY", cpkM, cpInteger, &delay, cpEnd) < 0) {
           	  return -1;
        }     
        return 0;
}
int Client::initialize(ErrorHandler *errh) {
	click_chatter("Initalize Errorhandler");
	ports_timer.initialize(this); //initializing the ports table 
        TimerTo_timer.initialize (this);//timer for retranmit
	ports_timer.schedule_after_sec(2);
       return 0;
}
void Client::run_timer(Timer* timer){
	click_chatter("Initializing timers");
	int dest;
	if(timer == &data_timer){
	//struct datapacket *format = (struct updatepacket *)p->data();
		if(myaddress == 1){
			data_generator();
			data_timer.schedule_after_sec(5);
			}
	}
          if (timer == &ports_timer) { 
		click_chatter("Build Ports table");
              build_portstable(); 
              ports_timer.schedule_after_sec(2);
        } else if (timer == &TimerTo_timer) { //retransmitting the packet
		//click_chatter("Cloning and resending it");
              Packet *pack = pkt.front()->clone();
              struct commonheader *header = (struct commonheader*) pack->data();
              click_chatter("Resending packet with seqnum = %d, retransmitting num = %d", header->seq_num, retransmissions++);
              output(0).push(pack);
              TimerTo_timer.reschedule_after_sec(time_out);   } 
}
void Client::data_generator()
{
	 click_chatter("Generate packet");
            int headroom = sizeof(click_ether);
            WritablePacket *packet = Packet::make(headroom,0,sizeof(struct datapacket)+ 8, 0);
            memset(packet->data(),0,packet->length());
            struct datapacket *format = (struct datapacket*) packet->data();
            format->header.type = DATA;
            format->header.seq_num = seqnum;
           format->header.src_address = uint16_t(myaddress);
            format->k= k;
           format->dst1 = uint16_t(dst1);
           format->dst2 = uint16_t(dst2);
           format->dst3 = uint16_t(dst3);
           format->payload = 8;
	    char *str = (char*)(packet->data()+sizeof(struct datapacket));
            memcpy(str, payload.c_str(), 8);
	   click_chatter("Data being sent :%s",str);
            pkt.push(packet);
            seqnum++; //increasiing the count by 1 every time the packet ispushed
            check_q(); //packet sent to the queue to find the status of the front packet of the queue
}
void Client::build_portstable() 
{
	   click_chatter("SENDING HELLO->Starting the build of the ports table");
            int headroom = sizeof(click_ether);
            WritablePacket *packet = Packet::make(headroom,0,sizeof(struct hellopacket), 0);
            memset(packet->data(),0,packet->length());
            struct hellopacket *format = (struct hellopacket*) packet->data();
            format->header.type = HELLO;
            format->header.src_address = uint16_t(myaddress);
            format->header.seq_num = seqnum;
            pkt.push(packet);
            click_chatter("HELLO with a seqnum = %u is queued", seqnum);
            seqnum++;
            check_q();
}
void Client::check_q() 
{
	//click_chatter("Check QueueStatus");
	int z=0;
	while(z==0){
		if (pkt.size()>0 ) { //check foe the packet
	             Packet *pack = pkt.front()->clone(); //clone the packeta and push it
        	     output(0).push(pack);
	             TimerTo_timer.schedule_after_sec(time_out);
        	     z = 1;
        		}
		}
}
void Client::push(int port, Packet *packet) {
	assert(packet);
      	if(port == 0) { //in case the packet was a hello packet-> send ack
	    struct datapacket *header = (struct datapacket *)packet->data();
           click_chatter("Received DATA packet \n seqnum = %u from %u", header->header.seq_num, header->header.src_address);   
            int headroom = sizeof(click_ether);
	    WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ackpacket), 0);
	    memset(ack->data(),0,ack->length());
            click_chatter("Sending ACK with seqnum = %u", header->header.seq_num);
            struct ackpacket *format = (struct ackpacket*) ack->data();
            format->header.type = ACK;
	    format->header.seq_num = header->header.seq_num; 
	    format->header.src_address = uint16_t(myaddress);
	    format->dst_address = header->header.src_address;
	    packet->kill(); 
            output(0).push(ack);
	}
	 else if (port == 1) { //in case the packet recever was an ackpacket
		struct ackpacket *header = (struct ackpacket *)packet->data();
		click_chatter("Received ACK of seqnum =  %d from %d", header->header.seq_num, header->header.src_address);
                WritablePacket *ackpkt = pkt.front();
                struct commonheader *ackheader = (struct commonheader*) ackpkt->data();
		if(header->header.seq_num == ackheader->seq_num) {
	                TimerTo_timer.unschedule(); 
                        pkt.pop(); //Pop it out of the queue if the ack was for the packet sent
			retransmissions=0; //No need to retransmit the data for that packet, instead kill it
                        packet->kill(); 
		}
	 else                      
		packet->kill();

	} 
	else {
              click_chatter("Unknown Packet -> drop");
		packet->kill(); 
	}
}
CLICK_ENDDECLS
EXPORT_ELEMENT(Client)