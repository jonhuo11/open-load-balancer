import socket
import threading
import argparse
import sys
from collections import defaultdict

# Global event to signal shutdown across all threads
shutdown_event = threading.Event()

# Shared dictionaries to track unique IPs and message counts for each port
ip_tracker = defaultdict(set)
message_counter = defaultdict(int)
tracker_lock = threading.Lock()

def listen_on_port(port: int) -> None:
    """
    Listens for UDP packets on the specified port and tracks unique IPs and message counts.

    Args:
        port (int): The port to listen for incoming UDP packets.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        # Set a timeout to periodically check the shutdown event
        sock.settimeout(1.0)
        sock.bind(('0.0.0.0', port))
        print(f"Listening for UDP packets on port {port}...")
        
        while not shutdown_event.is_set():
            try:
                # Use a timeout to allow periodic checking of shutdown_event
                data, addr = sock.recvfrom(1024)  # Buffer size of 1024 bytes
                client_ip = addr[0]
                
                # Thread-safe tracking of unique IPs and message count
                with tracker_lock:
                    ip_tracker[port].add(client_ip)
                    message_counter[port] += 1
                
                print(f"Received message from {addr}: {data.decode('utf-8', 'ignore')}")
            except socket.timeout:
                # This allows checking shutdown_event periodically
                continue
            except Exception as e:
                print(f"Error on port {port}: {e}")
                break

    # Print unique IPs and message count for this port when shutting down
    with tracker_lock:
        unique_ips = ip_tracker[port]
        message_count = message_counter[port]
        print(f"\nPort {port} Summary:")
        print(f"Total messages received: {message_count}")
        print(f"Unique IPs received: {len(unique_ips)}")
        for ip in sorted(unique_ips):
            print(f"  - {ip}")

def listen_on_multiple_ports(ports: list[int]) -> None:
    """
    Starts a listener on each port in the provided list, each in its own thread.

    Args:
        ports (List[int]): A list of ports to listen on.
    """
    threads = []
    
    for port in ports:
        thread = threading.Thread(target=listen_on_port, args=(port,))
        thread.daemon = True  # Ensure thread exits when main thread exits
        threads.append(thread)
        thread.start()

    # Wait for all threads to finish or be interrupted
    try:
        # Keep the main thread alive and periodically check if all threads are done
        while not shutdown_event.is_set():
            # Check if any thread is still alive
            if not any(thread.is_alive() for thread in threads):
                break
            shutdown_event.wait(1)
    except KeyboardInterrupt:
        print("\nInterrupting all listeners...")
    finally:
        # Ensure all threads are terminated
        shutdown_event.set()
        for thread in threads:
            thread.join(timeout=2)

def parse_ports_from_args() -> list[int]:
    """
    Parses the list of ports from command-line arguments.

    Returns:
        list[int]: A list of ports to listen on.
    """
    parser = argparse.ArgumentParser(description="Listen for UDP packets on specified ports.")
    parser.add_argument('ports', metavar='port', type=int, nargs='+', 
                        help='List of ports to listen on')
    args = parser.parse_args()
    return args.ports

def main():
    try:
        # Parse the ports from command-line arguments
        ports_to_listen = parse_ports_from_args()
        
        if not ports_to_listen:
            print("No ports specified. Please provide at least one port.")
            sys.exit(1)

        listen_on_multiple_ports(ports_to_listen)
    except KeyboardInterrupt:
        print("\nProgram terminated by user.")
    finally:
        # Ensure a final summary is printed
        print("\nOverall Summary:")
        for port in sorted(message_counter.keys()):
            unique_ip_count = len(ip_tracker[port])
            message_count = message_counter[port]
            print(f"Port {port}: {message_count} messages from {unique_ip_count} unique IPs")
        sys.exit(0)

if __name__ == "__main__":
    main()