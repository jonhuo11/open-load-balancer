# Open Load Balancer

A layer 4 UDP load balancer written in C++ to learn more about networking, Linux sockets, and C++ for performance.

Load balancer has client-server affinity but servers cannot respond to clients through the load balancer, they must implement DSR.

# Development stages

1) Server can read incoming byte data on a UDP socket (DONE)
2) Server can write out to 1 client on a UDP socket (DONE)
3) Server can write to multiple clients in round robin (DONE)
4) Server can take incoming IP address from header and route same IP to same client to preserve context (DONE)
5) Server is connected to services and if a service goes down it can adjust to it (WIP)
6) Command line interface for bringing up load balancer with settings, and updating load balancer configuration while it is running. This will require some basic threading (WIP)
7) A multithreaded implementation: load balancing should be done concurrently. The current implementation is pretty naive and needs to be updated
8) Profile single VS multithreaded versions
9) Clients that have not sent traffic in the last X mins should be removed from client-server mappings
10) Bidirectional UDP data: servers should be able to send UDP data back to the client they received the data from
11) Learn about ways to improve performance and implement them, perhaps by studying NGINX
12) Learn how to use concurrency library and see if this can be implemented (ideally multiple workers reading packets from a queue and doing work)
13) More algorithms and configuration settings

# Usage

Run `openloadbalancer [config_file]` to start the load balancer. This should open a command line interface to interact with the running load balancer. See `config.example` for a sample config file format.

Basic commands:
- help: shows a full list of commands
- quit: stop the load balancer

# Demo

There is demo code in `demopy`. Install requirements with `pip` and run using Python 3.

`traffic_gen.py` generates UDP packets and changes the IP header to be randomly selected from a provided list of IPs to simulate many outside clients making requests on the load balancer. This script uses `scapy` and requires root access to run.

`service.py` listens to UDP traffic on a list of ports and simulates the services that the load balancer is distributing work across.

Run the load balancer and set the service ports to the ports that `service.py` is listening on.