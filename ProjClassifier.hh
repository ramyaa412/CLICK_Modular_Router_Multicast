#ifndef CLICK_PROJCLASSIFIER_HH
#define CLICK_PROJCLASSIFIER_HH

#include <click/element.hh>
#include <click/timer.hh>

CLICK_DECLS

class ProjClassifier : public Element 
{

public:
	ProjClassifier();
	~ProjClassifier();

	const char *class_name() const { return "ProjClassifier";}
	const char *port_count() const { return "1/4"; }
	const char *processing() const { return "PUSH"; }

      int initialize(ErrorHandler*);
	void push(int, Packet*);
	

};

CLICK_ENDDECLS

#endif
