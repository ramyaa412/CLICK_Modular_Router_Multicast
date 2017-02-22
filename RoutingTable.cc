#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>
#include <click/packet_anno.hh>
#include "mypacket.hh"
#include "RoutingTable.hh"

CLICK_DECLS

RoutingTable::RoutingTable():router_timer(this), update_timer(this) {
//click_chatter("RoutingTable Constructor Initialized");
}
RoutingTable::~RoutingTable(){}
int RoutingTable::initialize(ErrorHandler *errh){
	click_chatter("Roouting table Timers initilaized");
    	router_timer.initialize(this);
   	update_timer.initialize(this);
	router_timer.schedule_after_sec(3);
    	update_timer.schedule_after_sec(6);
    	return 0;}
int RoutingTable::configure(Vector<String> &conf, ErrorHandler *errh) {
        if (cp_va_kparse(conf, this, errh,"TOPOLOGY", cpkP+cpkM, cpElement, &topo_ele, "MY_ADDRESS", cpkM, cpInteger, &myaddress, cpEnd) < 0) {
                  return -1;       }
            return 0;}

// Timer configurations 
void RoutingTable::run_timer(Timer* timer){
        if (timer == &router_timer) {
         	build_routertable();
         	router_timer.schedule_after_sec(2);
        }
        
        else if (timer == &update_timer) {
                update();
                update_timer.schedule_after_sec(5);
        }
}

//initialize routing table
void RoutingTable::build_routertable()//int port, Packet *p)
{		//start building the routing table

	std::map<int,int>&  port_table = topo_ele->get_port();
	for (porttable_iterator iter = port_table.begin(); iter != port_table.end(); iter++) //inserting the destination --cost --3 next hops into the routing table
        {
                click_chatter("Destination : %d \t Port : %d", iter->first, iter->second);
                routertable[iter->first][1].insert(iter->first);
        }
        routertable[myaddress][0].insert(0);
	click_chatter("\n\n ****** The Routing Table ***** \n\n");
   	 for ( outermap_iterator oit = routertable.begin() ; oit != routertable.end(); oit++ ) {
                click_chatter( "Destination : %d", (*oit).first) ;
                for(innermap_iterator iit=(*oit).second.begin(); iit != (*oit).second.end(); iit++){
                      click_chatter("Total Hop cost : %d", (*iit).first);          
             for(nexthop_iterator set_it=(*iit).second.begin(); set_it != (*iit).second.end(); set_it++){
                       click_chatter(" Next hops :%d", *set_it);
            }
         }  
    }

    click_chatter("\n-----------------------------------\n");
}
void RoutingTable::update(){
	int seq = topo_ele->get_seqnum();
	int headroom = sizeof(click_ether); //once the basic table is built, we start updating it with neighbouring costs to get the total hop count
	WritablePacket *packet = Packet::make(headroom,0,sizeof(struct updatepacket),0);
        if (packet == 0) 
		click_chatter( "No packet");
        struct updatepacket *update = (updatepacket *)packet->data();
        update->header.type = UPDATE;
        update->header.seq_num = seq;
        update->header.src_address = myaddress;
        update->routingpacket_length = sizeof(updatepacket);
        update->entries = routertable.size();
        int i = 0;
        for ( outermap_iterator oit = routertable.begin() ; oit != routertable.end(); oit++ ) { //run the outer map 
       		update->info[i].destination = (*oit).first ;             
                if ((routertable[(*oit).first].begin() != routertable[(*oit).first].end()) ) {
                	update->info[i].cost = routertable[(*oit).first].begin()->first;
                } i++;
         }
        click_chatter("Sending update with Sequence Number = %d forward", seq);
}
void RoutingTable::push(int port, Packet *packet) {
	assert(packet);
        struct commonheader *header = (struct commonheader *)packet->data();
        if (header->type == UPDATE) {  //checking if packet is update and working with it to update the cost
               click_chatter("Received update packet with seq num %d from %d", header->seq_num, header->src_address); 
        	struct updatepacket *update = (updatepacket *)packet->data();
        	int totalentries = int(update->entries);
         	for(int i=0; i < totalentries; i++) {
        	   click_chatter("  %d Cost %d\n",
                   update->info[i].destination,
	            update->info[i].cost);}

                for (int i=0; i<totalentries; i++) {
        // increase cost of every entry in the update packet by 1 
			int newcost = static_cast<int> (update->info[i].cost);
                        int updated_cost = int(++newcost);
                        int dest = int(update->info[i].destination);  
			int src = static_cast<int> (header->src_address);                  
 			if ( routertable.find(dest) == routertable.end() ) {
                                click_chatter("The Entry %d is not present. Adding to Routing Table", dest); //check if value is in entry table -> if not add it
                    		routertable[dest][updated_cost].insert(src);
			} 
			else {
                                      	if ((routertable[dest].begin() != routertable[dest].end()) ){     
	     				   if ((routertable[dest].begin()->first) > updated_cost ){
                                                 click_chatter("Lesser hop cost found. Updating table ");
                                                 routertable[dest].clear();
                                                 routertable[dest][updated_cost].insert(src);                                               	
        		       	   	   } 
					else if ((routertable[dest].begin()->first) <  updated_cost ){
                                                 click_chatter("Higher cost, Ignore");
                                  	   } 
					else if ((routertable[dest].begin()->first) ==  updated_cost ) {
				          		 if ( routertable[dest][(routertable[dest].begin()->first)].size() < 3 ){
                                                        	 routertable[dest][updated_cost].insert(src);
                                                                 click_chatter("Adding the three hops");
                                                         }
                                                 	else 
                                                        	 click_chatter("Table complete...");                                           }
   				 }   else {
                                            routertable[dest][updated_cost].insert(header->src_address); //the inner map maybe empty
                                 }
			}
                }


  // send ack for update packet
                int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ackpacket), 0);
                memset(ack->data(),0,ack->length());
                struct ackpacket *format = (struct ackpacket*) ack->data();
                        format->header.type = ACK;
                        format->header.seq_num = header->seq_num; 
                        format->header.src_address = uint16_t(myaddress);
                        format->dst_address = header->src_address;
              
                int dest = topo_ele->check_port(int(header->src_address));
                if (dest == -1){
                    click_chatter( "Packet not in ports table. Kill it.\n");
                    packet->kill();
               } 
		else {
	                    ack->set_anno_u8(8,dest);
        	            output(0). push(ack);
               		}
		}	 
		else {
	              click_chatter( "Wrong packet.\n");
        	      packet -> kill();
	        }
 	}

int RoutingTable::check_router(int hop){ //check for the router presence in the table
        if (routertable.find(hop) != routertable.end() ){
	        std::set<int> temp = routertable[hop].begin()->second;
                if (!temp.empty()) 
	              	return *temp.begin();
                } return -1;
}

std::map<int,std::map<int,std::set<int> > >& RoutingTable::get_router(){ //send the router table to forwarding element
  return routertable;}

CLICK_ENDDECLS
EXPORT_ELEMENT(RoutingTable)


