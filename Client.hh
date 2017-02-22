#ifndef CLICK_CLIENT_HH
#define CLICK_CLIENT_HH

#include <click/packet.hh>
#include <click/element.hh>
#include <click/timer.hh>
//#include <click/hashmaps.hh>

#include <vector>
#include <map>
#include <queue>

CLICK_DECLS

class Client : public Element {

public:
        Client();
	~Client();
	const char *class_name() const { return "Client";}
        const char *port_count() const { return "-/-"; }
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String>&, ErrorHandler*);
        int initialize(ErrorHandler*);

	void push(int, Packet *);
	void run_timer(Timer*);
        void build_portstable();
        void data_generator();
        void check_q();	

private:
	uint8_t retransmissions;
        uint8_t seqnum;
	uint8_t time_out; 
        int delay; 
        int myaddress; 
        int k;
        int dst1; 
        int dst2; 
        int dst3; 
        String payload; 
        Timer ports_timer; 
	Timer data_timer;
        Timer TimerTo_timer; 
        std::queue <WritablePacket *> pkt; 

};

CLICK_ENDDECLS

#endif