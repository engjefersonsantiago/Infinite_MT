from scapy.all import *
import os
import ipaddress
import random
import time


def generate_pcap_traces(num_five_tuples,num_pkt_per_tuple):
    # Init
    trace = list()
    trace_syn = list()
    random.seed(12)

    # Layer combination IPv4, IPv6, UDP, TCP 
    # Five Tuple Defined by a unique combinatio IP Layer Prot Src Port Dest Port
    # First time TCP packet SYN = 1, else SYN = 0
    IP_layer = ["IPv4", "IPv6"]
    Prot_layer = ["TCP", "UDP"]
    

    for five_tuple in range(num_five_tuples ):


        ip_prot = IP_layer[random.randrange(0,2)]
        if ip_prot == "IPv4":
            ip_add_src = str(ipaddress.IPv4Address(random.randrange(0,2**32)))
            ip_add_dst = str(ipaddress.IPv4Address(random.randrange(0,2**32)))
        else:
            ip_add_src = str(ipaddress.IPv6Address(random.randrange(0,2**128)))
            ip_add_dst = str(ipaddress.IPv6Address(random.randrange(0,2**128)))

        transport_prot = Prot_layer[random.randrange(0,2)]
        port_src = random.randrange(0,2**16)
        port_dst = random.randrange(0,2**16)

        for packet_index in range(num_pkt_per_tuple ):
            # Network Layer
            if ip_prot == "IPv4":
                packet = IP(dst=ip_add_dst,src= ip_add_src)
            else:
                packet = IPv6(dst=ip_add_dst,src= ip_add_src)

            # Payload - Random Padding Size 
            packet_padding = "load."*random.randrange(2,100)
        
            # Transport 
            if transport_prot == "TCP":
                # Not SYN 
                if packet_index:
                    packet = Ether() /packet / TCP(sport=port_src,dport=port_dst,flags="A")
                    packet = packet / packet_padding
                    trace.append(packet)
                # SYN
                else:
                    packet = Ether()/ packet / TCP(sport=port_src,dport=port_dst,flags="S")
                    packet = packet / packet_padding
                    trace_syn.append(packet)

            else:
                packet =  Ether()/ packet / UDP(sport=port_src,dport=port_dst)
                packet = packet / packet_padding
                trace.append(packet)    


    
    # Shuffle entries
    random.shuffle(trace)
    traces = trace_syn + trace
    print()

    # write trace
    file_name = f"test_{num_pkt_per_tuple}_pkts_{num_five_tuples}_tuples.pcap"


    # Del previous file if existing
    try:
        dir_path = os.path.dirname(os.path.realpath("generate_pcap.py"))
        file_path = dir_path + f"/{file_name}"
        os.remove(file_path)
    except FileNotFoundError:
        pass 

    for packet in traces:
        wrpcap(file_name,packet,append=True)

def generate_pcap_timestamp(num_tuples,num_packets): 
    
    file_name = f"test_{num_packets}_pkts_{num_tuples}_tuples_.times"
    total_pkts = num_tuples * num_packets
    
    # Max 10^-6 precision
    ref_time = time.time()
    
    with open(file_name, "w") as file_handler:
        for num_pkt in range(total_pkts + 1):
            ref_time = ref_time + random.randrange(0,10**6)/10**6
            if num_pkt != total_pkts:
                file_handler.write(f"{ref_time}\n")
            else:
                file_handler.write(f"{ref_time}")
      
            

if __name__ == "__main__":
    number_of_packets_per_tuple = 12
    number_of_tuples = 5
    generate_pcap_traces(number_of_tuples,number_of_packets_per_tuple)
    generate_pcap_timestamp(number_of_tuples,number_of_packets_per_tuple)