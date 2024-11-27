from collections import defaultdict
import random
import sys
import argparse
from scapy.all import IP, UDP, send

def send_random_udp_packet(
    target_ip: str,
    target_port: int,
    data: str,
    source_ips: list[str],
    packet_count: int = 10
) -> None:
    """
    Sends UDP packets with random source IP addresses and consistent source ports.

    Args:
        target_ip (str): The destination IP address for the UDP packet.
        target_port (int): The destination port for the UDP packet.
        data (str): The message data to send.
        source_ips (list[str]): List of source IP addresses to randomly choose from.
        packet_count (int): Number of packets to send.
    """
    send_summary = defaultdict(int)
    source_ports = {}  # Cache to store consistent ports for each source IP

    for _ in range(packet_count):
        # Select a random source IP
        source_ip = random.choice(source_ips)
        
        # Generate a consistent port for this source IP if not already existing
        if source_ip not in source_ports:
            source_ports[source_ip] = random.randint(1024, 65535)
        
        source_port = source_ports[source_ip]
        send_summary[source_ip] += 1
        
        # Create an IP layer with the random source IP
        ip = IP(src=source_ip, dst=target_ip)
        
        # Create a UDP layer with the consistent source port
        udp = UDP(sport=source_port, dport=target_port)
        
        # Create the packet with the IP and UDP layers, and the data
        packet_data = f"{data} from {source_ip}"
        packet = ip / udp / packet_data
        
        # Send the packet
        send(packet)
        print(f"Sent packet from {source_ip}:{source_port} to {target_ip}:{target_port} with data: {packet_data}")
    
    for ip in send_summary:
        print(f"{send_summary[ip]} packets sent from {ip}")

if __name__ == "__main__":
    # Rest of the code remains the same
    parser = argparse.ArgumentParser(description="Send UDP packets with random source IPs.")
    parser.add_argument("target_ip", help="Target IP address for the UDP packets.")
    parser.add_argument("target_port", type=int, help="Target port for the UDP packets.")
    parser.add_argument("data", nargs="?", default="Test message", help="The data to send in the UDP packets (default: 'Test message').")
    parser.add_argument("--source_ips", nargs='+', required=True, help="List of source IPs to choose from.")
    parser.add_argument("--packets", type=int, default=10, help="Number of packets to send (default: 10).")

    args = parser.parse_args()

    try:
        send_random_udp_packet(
            target_ip=args.target_ip,
            target_port=args.target_port,
            data=args.data,
            source_ips=args.source_ips,
            packet_count=args.packets
        )
    
    except KeyboardInterrupt:
        print("\nProgram terminated by user.")
        sys.exit(0)