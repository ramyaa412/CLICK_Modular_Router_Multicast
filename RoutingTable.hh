#ifndef CLICK_ROUTINGTABLE_HH
#define CLICK_ROUTINGTABLE_HH

#include <click/element.hh>
#include <click/vector.hh>
#include <click/timer.hh>
#include <click/hashmap.hh>

#include <algorithm>
#include <set>
#include <string>
#include <map>
#include <vector>

#include "Topology.hh"

CLICK_DECLS

class RoutingTable : public Element {
	
	public:
		RoutingTable();
		~RoutingTable();
		
		const char *class_name() const {return "RoutingTable";}
		const char *port_count() const {return "-/-";}
		const char *processing() const {return PUSH;}
			
	    void push(int port, Packet *packet);
            int configure(Vector<String>&, ErrorHandler*);
	    int initialize(ErrorHandler*);
	    void run_timer(Timer*); 
            void build_routertable();
	    void update();
	    	    
	    typedef std::map<int,int> porttable;
	    typedef std::map<int,int>::iterator porttable_iterator;         
	    typedef std::set<int> nexthop;
	    typedef std::map<int, nexthop> innermap;
       	    typedef std::map<int, innermap> mainmap;
	    typedef std::set<int>::const_iterator nexthop_iterator;
	    typedef std::map<int, nexthop>::const_iterator innermap_iterator;
	    typedef std::map<int, innermap>::const_iterator outermap_iterator;
	    mainmap& get_router();
	    int check_router(int);
       private:
            int myaddress;		
	    Timer router_timer;
	    Timer update_timer;
            mainmap routertable;
	    Topology *topo_ele;
};

CLICK_ENDDECLS
#endif


