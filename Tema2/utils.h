struct udp_message {
	char topic[50];
	unsigned char data_type;
	char payload[1500];
	char ip[16];
	int udp_port;
};

struct tcp_message {
	int type; //0-unsubscribe; 1-subscribe
	char topic[50];
	int flag; //0 sau 1
};
 
struct topic_struct
{
	char topic[50];
	int flag; //0 sau 1
};

struct client_struct
{
	char id[10];
	int contor;
	int socket;
	struct topic_struct* topics;
	int contor_msg;
	struct udp_message *udp_msg;	
};
