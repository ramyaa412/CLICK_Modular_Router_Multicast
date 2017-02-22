#ifndef CLICK_PACKETSWITCH_HH
#define CLICK_PACKETSWITCH_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <queue>

CLICK_DECLS

class PacketSwitch: public Element {

    public:
        PacketSwitch();
        ~PacketSwitch();
        const char *class_name() const { return "PacketSwitch";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);
        std::queue <Packet *> routerq[5]; 
};
CLICK_ENDDECLS
#endif