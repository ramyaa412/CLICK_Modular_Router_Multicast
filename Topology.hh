#ifndef CLICK_TOPOLOGY_HH
#define CLICK_TOPOLOGY_HH

#include <click/hashmap.hh>
#include <click/element.hh>
#include <click/timer.hh>

#include <map>

CLICK_DECLS

class Topology: public Element {

    public:

        Topology();
        ~Topology();

        const char *class_name() const { return "Topology";}
        const char *port_count() const { return "-/-";}
        const char *processing() const { return PUSH; }

        void build_portstable();
	void push(int port, Packet *packet);
        int initialize(ErrorHandler*);
	int configure(Vector<String>&, ErrorHandler*);
	void run_timer(Timer*); 
	typedef std::map<int, int> porttb;
        typedef std::map<int,int>::iterator portstable_iterator;
        int check_port(int);     
        porttb& get_port();
        int get_seqnum();

    private:
        int myaddress;
        int seqnum;
        int port_period;
        porttb portstable;
        Timer port_timer;
 };

CLICK_ENDDECLS
#endif
