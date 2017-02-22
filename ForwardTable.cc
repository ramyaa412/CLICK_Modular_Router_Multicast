#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/ip.h>
#include <click/packet.hh>

#include "ForwardTable.hh" 
#include "mypacket.hh"

CLICK_DECLS

ForwardTable::ForwardTable() {} //contructor
ForwardTable::~ForwardTable(){} //destructor

int ForwardTable::initialize(ErrorHandler *errh){
    return 0;
}
int ForwardTable::configure(Vector<String> &conf, ErrorHandler *errh) {
	//getting the data
	if (cp_va_kparse(conf, this, errh,"TOPOLOGY", cpkP+cpkM, cpElement, &topo_ele,"ROUTER", cpkP+cpkM, cpElement, &router_ele,"SWITCH", cpkP+cpkM, cpElement, &switch_ele,"MY_ADDRESS", cpkM, cpInteger, &myaddress,
 cpEnd) < 0) {
                  return -1;  }
        return 0;}
void ForwardTable::push(int port, Packet *packet) {
	assert(packet);
	struct commonheader *header = (struct commonheader *)(packet->data());
	if(header->type == ACK){   //checking if the packet was acknowledgement
		for (int index=0; index < 5; index++) { //assuming our queue takes only 5 packets at a time
                        if(switch_ele -> routerq[index].size()>0) { //send it to the switch element queue
			struct ackpacket *format = (struct ackpacket *)packet->data();
               		Packet* p = switch_ele -> routerq[index].front(); 
               		struct commonheader *pheader = (struct commonheader*) p->data();
			if(format-> header.seq_num == pheader->seq_num){
				switch_ele->routerq[index].pop();} //if the ack for the packet is same as the ack received -> pop it out from it
			}
		}
		
		struct ackpacket *format = (struct ackpacket *)packet->data();
		click_chatter("Received Ack at port %d", port); //Print the ack it was received at
		if(format->dst_address == uint16_t(myaddress))
			packet->kill();
	}
	else if(header->type == DATA){
		click_chatter("*******Received DATA- COMPLEXITY******");
		int headroom = sizeof(click_ether);
                WritablePacket *ack = Packet::make(headroom,0,sizeof(struct ackpacket), 0);
                memset(ack->data(),0,ack->length());
		struct ackpacket *format = (struct ackpacket *) ack->data();
		format->header.type = ACK;
		format->header.seq_num = header->seq_num;
                format->header.src_address = uint16_t(myaddress);
                format->dst_address = header->src_address;
		click_chatter("Source sent data from: %d", header->src_address);
		int src = static_cast<int> (header->src_address);
		int dst_port = topo_ele -> check_port(src); //get the apt port number for the source address
		while(dst_port != -1){
			click_chatter("ACK with Seq num %d \n to address %d", header->seq_num, src); 
			ack->set_anno_u8(8, dst_port);
			output(0).push(ack);}
		//ack->kill();
		struct datapacket *dpkt = (struct datapacket *)packet->data();
                click_chatter("*******DATA PACKET DETAILS******** "); //the data packet
		click_chatter("Source address %u \n Seq number  %u \n K %u \n Destination 1 %u\n Destination 2  %u \n Destination 3  %u", dpkt -> header.src_address, dpkt -> header.seq_num, dpkt ->k, dpkt -> dst1, dpkt -> dst2, dpkt -> dst3);
     		dpkt -> header.src_address = uint16_t(myaddress);
		int k = static_cast<int> (dpkt->k);
		int dst1 = static_cast<int>(dpkt -> dst1);
                int dst2 = static_cast<int>(dpkt -> dst2);
                int dst3 = static_cast<int> (dpkt ->dst3);
		if(k==1){  //when the K value is 1 ->find the shortest path
			int min;
			std::map<int,std::map<int,std::set<int> > >& router_tb = router_ele-> get_router(); //get the router table
			int dst1cost, dst2cost, dst3cost;
      			 if ( router_tb.find(dst1) != router_tb.end() ) {
			             dst1cost = router_tb[dst1].begin()->first;
             			click_chatter("Destination 1 = %d Cost = %d", dst1, dst1cost); //print the destination along with its cost
		        } else
		            	 dst1cost = 100; 
      			 if ( router_tb.find(dst2) != router_tb.end() ){ 
		             dst2cost = router_tb[dst2].begin()->first;
            			click_chatter("Destination 2 = %d Cost = %d", dst2, dst2cost);
		     } else 
		            dst2cost = 100; 
		       if ( router_tb.find(dst3) != router_tb.end() ) {
		             dst3cost = router_tb[dst3].begin()->first;
//             			click_chatter("Destination 3 = %d Cost = %d", dst3, dst3cost);
		       } else 
		            dst3cost = 100; 
			//int min;
		       if ((dst1cost < dst2cost) && (dst1cost < dst3cost))
               		    min = dst1; //if destination 1 is reached in the smallest cost
		       else if ((dst2cost < dst1cost) && (dst2cost < dst3cost))
		            min = dst2;
		       else if ((dst3cost < dst1cost) && (dst3cost < dst2cost))
		              min = dst3;
			int min_dst = min;
			int nexthop_router = router_ele -> check_router(min_dst); //find the apt next router using the minimum destination
			int nexthop_port = topo_ele -> check_port(nexthop_router); //for the above destination find the port
			if(nexthop_router == -1 || nexthop_port == -1){ //if that destination is not present
				click_chatter("Destination missing");
				packet->kill();
			}
			else{
				packet->set_anno_u8(8, nexthop_port); //else just push it
				output(0).push(packet);
			}
		}
	else if(k==2){ //if k value is2
		if((dst1 == 0 && dst2 == 0) || (dst1 == 0 && dst3 == 0)|| (dst2 ==0 && dst3 == 0)){ //if 2 out of the 3 destinations are absent
			click_chatter("2 destinations are missing");
			packet-> kill();
		}
		
		std::map<int,std::map<int,std::set<int> > >& router_tb = router_ele-> get_router();
			int dst1cost, dst2cost, dst3cost, min, min2;
      			 if ( router_tb.find(dst1) != router_tb.end() ) {
			             dst1cost = router_tb[dst1].begin()->first; 
             				click_chatter("Destination 1 = %d Cost = %d", dst1, dst1cost);
			}
		        else
		             dst1cost = 100; 
      			 if ( router_tb.find(dst2) != router_tb.end() ) {
		             dst2cost = router_tb[dst2].begin()->first;
            			click_chatter("Destination 2 = %d Cost = %d", dst2, dst2cost);
		    }  else 
		            dst2cost = 100; 
		       if ( router_tb.find(dst3) != router_tb.end() ) {
		             dst3cost = router_tb[dst3].begin()->first;
		             click_chatter("Destination 3 = %d Cost = %d", dst3, dst3cost);
		       }
			 else 
		            dst3cost = 100; 
			//find the first minimum cost
		       if ((dst1cost < dst2cost) && (dst1cost < dst3cost))
               		    min = dst1;
		       else if ((dst2cost < dst1cost) && (dst2cost < dst3cost))
		            min = dst2;
		       else if ((dst3cost < dst1cost) && (dst3cost < dst2cost))
		              min = dst3;
			//looking for second least hop->check if its smaller hop count than other two while the destiantion is bigger than the minimum destination and not equal to it
			if ((dst1cost < dst2cost) && (dst1cost < dst3cost) && (dst1 > min) && (dst1 != min))
				min2 = dst1;
			else if ((dst2cost < dst1cost) && (dst2cost < dst3cost) && (dst2 > min) && (dst2 != min))
		                min2 = dst2;
			else if ((dst3cost < dst1cost) && (dst3cost < dst2cost) && (dst3 > min) && (dst3 != min))
		              min2 = dst3;
			std::set<int> min1_hops;
   			std::set<int> min2_hops;
		      if ((router_tb[min].begin() != router_tb[min].end()) ){
		        if (router_tb[min][router_tb[min].begin()->first].size() > 0 ){
         		   min1_hops = router_tb[min].begin()->second;
       			 }
      			}   
		    if ((router_tb[min2].begin() != router_tb[min2].end()) ){
        		if ( router_tb[min2][router_tb[min2].begin()->first].size() > 0 ){
            			min2_hops = router_tb[min2].begin()->second;
        		}
        	    }
			std::set<int> intersect; //start looking for a common hop
			int commonhop;
			set_intersection(min1_hops.begin(), min1_hops.end(),
					 min2_hops.begin(), min2_hops.end(),
					 std::inserter(intersect, intersect.begin()));
			std::set<int>::iterator it;
			it = intersect.begin();
			if(intersect.begin() != intersect.end())
				commonhop = *intersect.begin();
			else
				commonhop = -1;
			if(commonhop == -1){
				int nexthop1_router = router_ele -> check_router(min); //incase no common hop is present. Make copies and send the packets 
				int nexthop1_port = topo_ele -> check_port(nexthop1_router); //find the next hop and the router for the reqd destination
				int nexthop2_router = router_ele -> check_router(min2);
				int nexthop2_port = topo_ele -> check_port(nexthop2_router);
				Packet *send1 = packet -> clone();
				struct datapacket *dpkt1 = (struct datapacket *)send1->data();
				dpkt1 -> k =1;
				dpkt1 -> dst1 = uint16_t(min);
				dpkt1 -> dst2 = 0;
				dpkt1 -> dst3 = 0;
				send1 -> set_anno_u8(8, nexthop1_port);
				click_chatter("Sending Copy-1 to %d with next hop %d", min , nexthop1_port); //send the packet
				output(0).push(send1); 
				Packet *send2 = packet -> clone();
				//WritablePacket *send2 = temp_pkt -> uniqueify();
				struct datapacket *dpkt2 = (struct datapacket *)send2->data();
				dpkt2 -> k =1;
				dpkt2 -> dst1 = uint16_t(min2);
				dpkt2 -> dst2 = 0;
				dpkt2 -> dst3 = 0;
				send2 -> set_anno_u8(8, nexthop2_port);
				click_chatter("Sending Copy-2 to %d with next hop %d", min2 , nexthop2_port);
				output(0).push(send2); 
				}
			else {
				int nexthop_port = topo_ele ->check_port(commonhop); //send one packet to commonhop
				packet -> set_anno_u8(8, nexthop_port); //paint it and forward
				click_chatter("Packet sent to Common hop");
				output(0).push(packet);
				}
		
	}
		else if(k ==3){
			if (dst1 == 0 || dst2 == 0 || dst3 == 0){
				click_chatter("One or more destinations are absent"); //if even one destination is absent
				packet->kill();
				}
			
				int commonhop_3dst;
				std::map<int,std::map<int,std::set<int> > >& router_tb = router_ele-> get_router();
				std::set<int> dst1_hops;
   				std::set<int> dst2_hops;
 			        std::set<int> dst3_hops;
				 if ((router_tb[dst1].begin() != router_tb[dst1].end()) ){
				        if ( router_tb[dst1][router_tb[dst1].begin()->first].size() > 0 ){
				            dst1_hops = router_tb[dst1].begin()->second;
					        }
				    }
				   if ((router_tb[dst2].begin() != router_tb[dst2].end()) ){
				        if (router_tb[dst2][router_tb[dst2].begin()->first].size() > 0 ){
				            dst2_hops = router_tb[dst2].begin()->second;
				        }
				    }
				    if ((router_tb[dst3].begin() != router_tb[dst3].end()) ){
				        if (router_tb[dst3][router_tb[dst3].begin()->first].size() > 0 ){
				            dst3_hops = router_tb[dst3].begin()->second;
				        }
				    }

				  std::set<int> intersect1;
      				  std::set<int> intersect2;
  				  std::set_intersection(dst1_hops.begin(),dst1_hops.end(),
							dst2_hops.begin(),dst2_hops.end(),
							std::inserter(intersect1,intersect1.begin()));
			      	  std::set_intersection(dst3_hops.begin(),dst3_hops.end(),
							intersect1.begin(),intersect1.end(),
							std::inserter(intersect2,intersect2.begin()));
				  std::set<int>::iterator it;
				it = intersect2.begin();
    			    	if(intersect2.begin()!= intersect2.end()) //Looking for a common hop for all three
					commonhop_3dst = *intersect2.begin();
				else 
					commonhop_3dst = -1;

			if (commonhop_3dst == -1){ 
				click_chatter(" No common hop present for all three destinations");
				int commonhop_dst12;  //no common hop for all three
				std::set<int>::iterator iit ;
				iit = intersect1.begin();
   			    	if(intersect1.begin() != intersect1.end())
					commonhop_dst12 = *intersect1.begin(); //check common hop for dest 1 and 2
				else
					commonhop_dst12 = -1;
				if(commonhop_dst12 == -1){  //No common hop for dest 1 and 2 ..... looking for common hop for dest 2 & 3
					int commonhop_dst23;
					click_chatter("No Common hop for First 2 destinations");
					std::set<int> intersect23;
  				  	std::set_intersection(dst2_hops.begin(),dst2_hops.end(),
							      dst3_hops.begin(),dst3_hops.end(),
							      std::inserter(intersect23,intersect23.begin()));
					std::set<int>::iterator it;
					it = intersect23.begin();
					if(intersect23.begin() != intersect23.end())
						commonhop_dst23 = *intersect23.begin();
					else
						commonhop_dst23 = -1;
				if (commonhop_dst23 == -1){  //no common hop for last 2
					click_chatter("No Common hop for last 2");
					int commonhop_dst13;  //searching common hops for dest 1 and 3
					std::set<int> intersect13;
  				  	std::set_intersection(dst1_hops.begin(),dst1_hops.end(),
							      dst3_hops.begin(),dst3_hops.end(),
							      std::inserter(intersect13,intersect13.begin()));
					std::set<int>::iterator it;
					it = intersect13.begin();
					if(intersect13.begin() != intersect13.end())
						commonhop_dst13 = *intersect13.begin();
					else
						commonhop_dst13 = -1;
				if (commonhop_dst13 == -1) { //No commonhops
					click_chatter("No common hop for dest 1 and 3 \nNo Common hops at all \n Sending 3 packets separately");
					int nexthop1_router = router_ele -> check_router(dst1);
					int nexthop1_port = topo_ele -> check_port(nexthop1_router);
					int nexthop2_router = router_ele -> check_router(dst2);
					int nexthop2_port = topo_ele -> check_port(nexthop2_router);
					int nexthop3_router = router_ele -> check_router(dst3);
					int nexthop3_port = topo_ele -> check_port(nexthop3_router);
					Packet *send1 = packet -> clone();
					struct datapacket *dpkt1 = (struct datapacket *)send1->data();
					dpkt1 -> k =1;
					dpkt1 -> dst1 = uint16_t(dst1);
					dpkt1 -> dst2 = 0;
					dpkt1 -> dst3 = 0;
					send1 -> set_anno_u8(8, nexthop1_port);
					click_chatter("Sending Copy-1 to %d with next hop %d", dst1 , nexthop1_port);
					if(nexthop1_port == -1)
						send1->kill();
					else
						output(0).push(send1); 
					Packet *send2 = packet -> clone();
					struct datapacket *dpkt2 = (struct datapacket *)send2->data();
					dpkt2 -> k =1;
					dpkt2 -> dst1 = uint16_t(dst2);
					dpkt2 -> dst2 = 0;
					dpkt2 -> dst3 = 0;
					send2 -> set_anno_u8(8, nexthop2_port);
					click_chatter("Sending Copy-2 to %d with next hop %d", dst2 , nexthop2_port);
					if(nexthop2_port == -1)
						send2->kill();
					else
						output(0).push(send2); 
					Packet *q = packet -> clone();
					Packet *send3 = packet -> clone();
					struct datapacket *dpkt3 = (struct datapacket *)send3->data();
					dpkt3 -> k =1;
					dpkt3 -> dst1 = uint16_t(dst3);
					dpkt3 -> dst2 = 0;
					dpkt3 -> dst3 = 0;
					send3 -> set_anno_u8(8, nexthop3_port);
					click_chatter("Sending Copy-3 to %d with next hop %d", dst3 , nexthop2_port);
					if(nexthop3_port == -1)
						send3->kill();
					else
						output(0).push(send3);
					//temp_pkt -> kill();
				}
				else {
					click_chatter("Common hop found for %d and %d", dst1, dst3); //COmmon hop for dest 1 and 3
					int nexthop13_router = commonhop_dst13;
					int nexthop13_port = topo_ele -> check_port(nexthop13_router);
					int nexthop2_router = router_ele -> check_router(dst2);
					int nexthop2_port = topo_ele -> check_port(nexthop2_router);
					Packet *send1 = packet -> clone();		
					struct datapacket *dpkt1 = (struct datapacket *)send1->data();
					dpkt1 -> k =2;
					dpkt1 -> dst1 = uint16_t(dst1);
					dpkt1 -> dst2 = uint16_t(dst3);
					dpkt1 -> dst3 = 0;
					send1 -> set_anno_u8(8, nexthop13_port);
					click_chatter("Sending Copy-1 to %d, %d with next hop %d", dst1 , dst3 , nexthop13_port); //same packet sent to common hopfor dest 1 and 3
					output(0).push(send1); 
					Packet *send2 = packet -> clone();
					struct datapacket *dpkt2 = (struct datapacket *)send2->data();
					dpkt2 -> k =1;
					dpkt2 -> dst1 = uint16_t(dst2);
					dpkt2 -> dst2 = 0;
					dpkt2 -> dst3 = 0;
					send2 -> set_anno_u8(8, nexthop2_port);
					click_chatter("Sending Copy-2 to %d with next hop %d", dst2 , nexthop2_port); //One separate packet sent to destination 2
					output(0).push(send2); 
					}
			}
			else{
				click_chatter("Common hop found for %d and %d",dst2,dst3);
				int nexthop23_router = commonhop_dst23;
				int nexthop23_port = topo_ele -> check_port(nexthop23_router);
				int nexthop1_router = router_ele -> check_router(dst1);
				int nexthop1_port = topo_ele -> check_port(nexthop1_router);
				Packet *send1 = packet -> clone();		
				//WritablePacket *send1 = packet->uniqueify();
				struct datapacket *dpkt1 = (struct datapacket *)send1->data();
				dpkt1 -> k =2;
				dpkt1 -> dst1 = uint16_t(dst2);
				dpkt1 -> dst2 = uint16_t(dst3);
				dpkt1 -> dst3 = 0;
				send1 -> set_anno_u8(8, nexthop23_port);
				click_chatter("Sending Copy-1 to %d, %d with next hop %d", dst2 , dst3 , nexthop23_port);
				output(0).push(send1); 
				//WritablePacket *send2 = temp_pkt -> uniqueify();
				Packet *send2 = packet -> clone();
				struct datapacket *dpkt2 = (struct datapacket *)send2->data();
				dpkt2 -> k =1;
				dpkt2 -> dst1 = uint16_t(dst1);
				dpkt2 -> dst2 = 0;
				dpkt2 -> dst3 = 0;
				send2 -> set_anno_u8(8, nexthop1_port);
				click_chatter("Sending Copy-2 to %d with next hop %d", dst1 , nexthop1_port);
				output(0).push(send2); 
				}
			}
			else {
				click_chatter("Common hop found for %d and %d",dst1,dst2);
				int nexthop12_router = commonhop_dst12;
				int nexthop12_port = topo_ele -> check_port(nexthop12_router);
				int nexthop3_router = router_ele -> check_router(dst3);
				int nexthop3_port = topo_ele -> check_port(nexthop3_router);
				Packet *send1 = packet -> clone();		
				//WritablePacket *send1 = packet->uniqueify();
				struct datapacket *dpkt1 = (struct datapacket *)send1->data();
				dpkt1 -> k =2;
				dpkt1 -> dst1 = uint16_t(dst1);
				dpkt1 -> dst2 = uint16_t(dst2);
				dpkt1 -> dst3 = 0;
				send1 -> set_anno_u8(8, nexthop12_port);
				click_chatter("Sending Copy-1 to %d, %d with next hop %d", dst1 , dst2 , nexthop12_port);
				output(0).push(send1); 
				//WritablePacket *send2 = temp_pkt -> uniqueify();
				Packet *send2 = packet -> clone();
				struct datapacket *dpkt2 = (struct datapacket *)send2->data();
				dpkt2 -> k =1;
				dpkt2 -> dst1 = uint16_t(dst3);
				dpkt2 -> dst2 = 0;
				dpkt2 -> dst3 = 0;
				send2 -> set_anno_u8(8, nexthop3_port);
				click_chatter("Sending Copy-2 to %d with next hop %d", dst3 , nexthop3_port);
				output(0).push(send2); 
				}
			}
			else {
				click_chatter(" Found Common Hop ");  //No need to clone packets-> One packet sent to common hop
                                int nexthop_port = topo_ele -> check_port(commonhop_3dst);
                                packet->set_anno_u8(8, nexthop_port);
                                click_chatter("Sending to  %d through port %d", commonhop_3dst, nexthop_port);
                                output(0).push(packet);     
				}
			}else {
                    
                   	  click_chatter("Error: Invalid value of k");
                }
		}
		 else {
	            click_chatter(" Wrong packet type received.");
	            packet->kill();
        }
}


CLICK_ENDDECLS
EXPORT_ELEMENT(ForwardTable)

						
			



    
