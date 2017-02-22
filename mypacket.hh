#ifndef CLICK_MYPACKET_HH
#define CLICK_MYPACKET_HH

typedef enum {
	HELLO = 1,
	UPDATE,
	ACK,
	DATA 
} packet_types;

struct commonheader{
	uint8_t type;
	uint8_t seq_num;
	uint16_t src_address;
};

struct routinginfo{
	uint16_t destination; 
  	uint8_t cost;  

};
struct datapacket{
	struct commonheader header;
	uint8_t k;
	uint16_t dst1;
	uint16_t dst2;
	uint16_t dst3;
     uint16_t payload;
};

struct hellopacket
	{
	struct commonheader header;
	
	};

struct ackpacket
	{
	struct commonheader header;
        uint16_t dst_address;
		
};

struct updatepacket
	{
  	struct commonheader header; 
        uint16_t routingpacket_length;
     	struct routinginfo info[100];
        uint16_t entries;
		
};

#endif