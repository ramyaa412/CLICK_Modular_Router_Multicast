#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/packet.hh>

#include "ProjClassifier.hh"
#include "mypacket.hh" 

CLICK_DECLS

ProjClassifier::ProjClassifier() {click_chatter("Classifier Constrcutor");}
ProjClassifier::~ProjClassifier(){}
int ProjClassifier::initialize(ErrorHandler *errh){
	click_chatter("Classifier Errorhandler");
    return 0;
}
void ProjClassifier:: push(int port, Packet *packet){
		click_chatter("Classifier push");
		assert(packet);
		struct commonheader *header = (struct commonheader *)packet->data();
		if(header->type == 1) {
			output(0).push(packet); //hello
		}
		 else if (header->type == 2) {             
			output(1).push(packet);//update
		} else if(header->type == 3) {               
			output(2).push(packet); //ack
		} else if(header->type == 4) {                             
			 output(3).push(packet);//data
	        } else {		
			packet->kill();
	}
}
CLICK_ENDDECLS
EXPORT_ELEMENT(ProjClassifier)
