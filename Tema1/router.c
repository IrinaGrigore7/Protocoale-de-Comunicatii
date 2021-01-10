#include "skel.h"
#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* pentru open(), exit() */
#include <fcntl.h> /* O_RDWR */
#include <errno.h> /* perror() */

/*calculeaza lungimea pe care ar trebui sa-o aiba fiecare tabela */
int get_size(char *file_name){
	int fd = open(file_name, O_RDONLY);
	if(fd < 0)
    	perror("Can't open the file");
	int len_file = lseek(fd, 0, SEEK_END);
	char* buf = malloc(len_file);
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, len_file);
	int size = 0;
	const char s[2] = "\n";
    char *token;
    token = strtok(buf, s);
    while(token != NULL){
    	size++;
    	token = strtok(NULL, s);
    }
    close(fd);
	return size;
}

/*gaseste intrarea in tabela arp*/
struct arp_entry *get_arp_entry(struct arp_entry *arp_table, int len_arp, __u32 ip) {
    for(int i = 0; i < len_arp - 1; i++){
    	if(arp_table[i].ip == ip)
    		return &arp_table[i];
    }
    return NULL;
}

/*citeste din fisierul text si completeaza tabela de rutare;
pun continutul fisierului intr-un buffer, il separ dupa saptiu si "\n
si apoi completez campurile corespunzatoare in tabela.*/
int read_rtable(struct route_table_entry *rtable){
	int fd = open("rtable.txt", O_RDONLY);
	if(fd < 0)
    	perror("Can't open the file");
	int len_file = lseek(fd, 0, SEEK_END);
	char* buf = malloc(len_file);
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, len_file);
	int len_rtable = get_size("rtable.txt");
	struct route_table_entry rtable1;
    const char s[3] = " \n";
    char *token = strtok(buf, s);
	int index = 0;
	int count = 0;

	while(token != NULL){
		index++;
		if(index == 1)
			rtable1.prefix = inet_addr(token);
		if(index == 2)
			rtable1.next_hop = inet_addr(token);
		if(index == 3)
			rtable1.mask = inet_addr(token);
		if(index == 4){
			rtable1.interface = atoi(token);
			rtable[count] = rtable1;
			count++;
			index = 0;
		}
		token = strtok(NULL, s);
	} 
	close(fd);
	return len_rtable;
}

/*citeste din fisier si completeza campurile din tabela arp*/
int arp_rtable(struct arp_entry *arp_table){
	int fd = open("arp_table.txt", O_RDONLY);
	if(fd < 0)
    	perror("Can't open the file");
	int len_file = lseek(fd, 0, SEEK_END);
	char* buf = malloc(len_file);
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, len_file);
	int len_rtable = get_size("arp_table.txt");
	struct arp_entry arp_table1;
    const char s[3] = " \n";
    char *token = strtok(buf, s);
	int index = 0;
	int count = 0;

	while(token != NULL){
		index++;
		if(index == 1)
			arp_table1.ip = inet_addr(token);
		if(index == 2){ 
			hwaddr_aton(token,arp_table1.mac);
			arp_table[count] = arp_table1;
			count++;
			index = 0;
		}
		token = strtok(NULL, s);
	} 
	close(fd);
	return len_rtable;
}


void merge(struct route_table_entry *rtable, int left, int mid, int right) 
{ 
    int i, j, k; 
    int n1 = mid - left + 1; 
    int n2 = right - mid; 
  
    struct route_table_entry *L, *R; 
    L = malloc(sizeof(struct route_table_entry)*n1);
    R = malloc(sizeof(struct route_table_entry)*n2);
  
    for (i = 0; i < n1; i++) 
        L[i] = rtable[left + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = rtable[mid + 1+ j]; 
  
    i = 0; 
    j = 0;
    k = left; 
    while (i < n1 && j < n2) { 
        if ((uint32_t)L[i].prefix < (uint32_t)R[j].prefix) { 
            rtable[k] = L[i]; 
            i++; 
        } 
        else{ 
            rtable[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
 
    while (i < n1) 
    { 
        rtable[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    while (j < n2){ 
        rtable[k] = R[j]; 
        j++; 
        k++; 
    } 
} 

/*functie de sortare; sortez tabela crescator in functie de prefix 
 si descrascator dupa masca; aceasta se realizeaza in O(nlogn)*/
void mergeSort(struct route_table_entry *rtable, int left, int right) 
{ 
    if (left < right) { 
        int mid = left + (right - left) / 2; 
  
        mergeSort(rtable, left, mid); 
        mergeSort(rtable, mid + 1, right); 
  
        merge(rtable, left, mid, right); 
    } 
} 

/*caut in tabela de rutare pozitia la care se afla prefixul cautat; cautarea se face in O(logn)*/
int binarySearch(struct route_table_entry *rtable, int l, int r, uint32_t dest_ip) 
{ 
    if (r >= l) { 
        int mid = l + (r - l) / 2; 

        if ((rtable[mid].mask & dest_ip) == rtable[mid].prefix) 
            return mid; 
  
        if ((rtable[mid].mask & dest_ip) < rtable[mid].prefix) 
            return binarySearch(rtable, l, mid - 1, dest_ip); 
  
        return binarySearch(rtable, mid + 1, r, dest_ip); 
    } 
 
    return -1; 
} 
 /*dupa ce obtin pozitia lacare se afla prefixul cautat in tabela, o caut pe cea mai specifica*/
struct route_table_entry *get_best_route( struct route_table_entry *rtable, int rtable_size, __u32 dest_ip) {
	int crt = binarySearch(rtable, 0, rtable_size - 1, dest_ip);
	int mid = crt;
	while(rtable[crt].prefix == rtable[mid].prefix)
		mid--;
	if(crt == -1)
		return NULL;
	return &rtable[mid];
}

/*calculeaza checksum-ul ip-ului*/
uint16_t ip_checksum(void* vdata,size_t length) {
	char* data = (char*)vdata;

	uint64_t acc = 0xffff;

	unsigned int offset = ((uintptr_t)data) & 3;
	if (offset) {
		size_t count = 4 - offset;
		if (count > length) count = length;
		uint32_t word = 0;
		memcpy(offset + (char*)&word, data,count);
		acc += ntohl(word);
		data += count;
		length -= count;
	}

	char* data_end = data + (length&~3);
	while (data != data_end) {
		uint32_t word;
		memcpy(&word, data, 4);
		acc += ntohl(word);
		data += 4;
	}
	length &= 3;

	if (length) {
		uint32_t word = 0;
		memcpy(&word, data, length);
		acc += ntohl(word);
	}

	acc = (acc&0xffffffff) + (acc>>32);
	while (acc>>16) {
		acc = (acc&0xffff) + (acc>>16);
	}

	if (offset&1) {
		acc = ((acc&0xff00)>>8) | ((acc&0x00ff)<<8);
	}

	return htons(~acc);
}

/*seteaza campurile header-ului de ip*/
void set_iphdr(packet pack, struct iphdr *ip_hdr, uint32_t ip_addr, struct iphdr *ip_hdr1 ){
	ip_hdr1->version = 4;
	ip_hdr1->ihl = 5;
	ip_hdr1->tos = 0;
	ip_hdr1->tot_len = htons(pack.len - sizeof(struct ether_header));
	ip_hdr1->id = htons(25);
	ip_hdr1->frag_off = htons(0);
	ip_hdr1->ttl = ip_hdr->ttl;
	ip_hdr1->protocol = IPPROTO_ICMP;
	ip_hdr1->saddr = ip_addr;
	ip_hdr1->daddr = ip_hdr->saddr;
	ip_hdr1->check = 0;
	ip_hdr1->check = ip_checksum(ip_hdr1, sizeof(struct iphdr));
} 

/*seteaza campurile header-ului de ethernet*/
void set_ether_header(packet pack, uint32_t ip_addr, struct ether_header *eth_hdr1, struct ether_header *eth_hdr){
	memcpy(eth_hdr1->ether_dhost, eth_hdr->ether_shost, sizeof(eth_hdr->ether_shost));
	memcpy(eth_hdr1->ether_shost, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));
	eth_hdr1->ether_type = htons(ETHERTYPE_IP);
}

/*seteaza campurile header-ului de icmp cu type corespunzator*/
void set_icmphdr(packet pack, struct icmphdr *icmp_hdr1, u_int8_t type){
	icmp_hdr1->code = 0;
	icmp_hdr1->type = type;
	icmp_hdr1->un.echo.id = 0;
	icmp_hdr1->un.echo.sequence = 0;
	icmp_hdr1->checksum = 0;
	icmp_hdr1->checksum = ip_checksum(icmp_hdr1, sizeof(struct icmphdr));
}

/*apelez functiile de completare a headerelor de ethernet/ip/icmp, apoi trimit pachetul pe interfata corespunzatoare*/
void set_headers(packet m, struct ether_header *eth_hdr, struct iphdr *ip_hdr, u_int8_t type){
	uint32_t ip_addr = inet_addr(get_interface_ip(m.interface));
	packet pack;
	pack.len = sizeof(struct ether_header) + sizeof(struct iphdr)
										+ sizeof(struct icmphdr);
	pack.interface = m.interface;
	struct ether_header *eth_hdr1 = (struct ether_header *)(pack.payload);
	set_ether_header(pack, ip_addr, eth_hdr1, eth_hdr);

	struct iphdr *ip_hdr1 = (struct iphdr *)(pack.payload + sizeof(struct ether_header));
	set_iphdr(pack, ip_hdr, ip_addr, ip_hdr1);
						
	struct icmphdr *icmp_hdr1 = (struct icmphdr *)(pack.payload + 
						sizeof(struct ether_header) + sizeof(struct iphdr));
	set_icmphdr(pack, icmp_hdr1, type);
						
	send_packet(m.interface, &pack);
	return;

}

int main(int argc, char *argv[])
{
	packet m;
	int rc;	
	struct route_table_entry * var;
	init();

	int len_rtable = get_size("rtable.txt");
	struct route_table_entry *rtable =  malloc(sizeof(struct route_table_entry)*len_rtable);
	read_rtable(rtable); //construiesc tabela de rutare
	
	mergeSort(rtable, 0, len_rtable - 1); 
	int len_arp = get_size("arp_table.txt");
	struct arp_entry *arp_table = malloc(sizeof(struct arp_entry)*len_arp);
	arp_rtable(arp_table);//construiesc tabela arp 
	 
	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));
		struct icmphdr *icmp_hdr = (struct icmphdr *)(m.payload + 
							sizeof(struct ether_header) + sizeof(struct iphdr));
		
		if(eth_hdr->ether_type == htons(ETHERTYPE_IP)){
			if(ip_hdr->protocol == IPPROTO_ICMP){
				if(icmp_hdr->type == ICMP_ECHO){
					uint32_t ip_addr = inet_addr(get_interface_ip(m.interface));
					if(ip_addr == ip_hdr->daddr){
						set_headers(m, eth_hdr, ip_hdr, ICMP_ECHOREPLY);
						continue;
					}
				}
			}
			if(ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0)
				continue;
			if(ip_hdr->ttl <= 1){
				set_headers(m, eth_hdr, ip_hdr, ICMP_TIME_EXCEEDED);
				continue;
				
			}

			var = get_best_route(rtable, len_rtable, ip_hdr->daddr);
			if(var == NULL){
				set_headers(m, eth_hdr, ip_hdr, ICMP_DEST_UNREACH);
				continue;	
			}
			ip_hdr->ttl--;
			ip_hdr->check = 0;
			ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));
			struct arp_entry * var2 = get_arp_entry(arp_table, len_arp,ip_hdr->daddr);
			if(var2 == NULL)
				continue;
			memcpy(eth_hdr->ether_dhost, var2->mac, 6);
			get_interface_mac(var->interface,eth_hdr->ether_shost);
			send_packet(var->interface, &m);
		}
	}
}
