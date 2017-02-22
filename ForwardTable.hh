#ifndef CLICK_FORWARDTABLE_HH
#define CLICK_FORWARDTABLE_HH

#include <click/hashmap.hh>
#include <click/element.hh>
#include <click/timer.hh>
#include "Topology.hh"
#include "PacketSwitch.hh"
#include "RoutingTable.hh"
#include <vector>
CLICK_DECLS

class ForwardTable : public Element
{
	public:
		ForwardTable();
		~ForwardTable();
		const char *class_name() const { return "ForwardTable";}
		const char *port_count() const { return "-/-"; }
		const char *processing() const { return PUSH; }
		 
		void push(int port, Packet *packet);
	        int initialize(ErrorHandler*);
		int configure(Vector<String>&, ErrorHandler*);
		typedef  std::set<int>::iterator it;

    private:

        int myaddress;
        Topology* topo_ele;
        RoutingTable* router_ele;
        PacketSwitch* switch_ele;

};

CLICK_ENDDECLS
#endif