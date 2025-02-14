# Open Load Balancer

A layer 4 UDP load balancer written in C++ to learn more about networking, Linux sockets, and C++ for performance.

# Development stages

1) Server can read incoming byte data on a UDP socket (DONE)
2) Server can write out to 1 client on a UDP socket (DONE)
3) Server can write to multiple clients in round robin (DONE)
4) Server can take incoming IP address from header and route same IP to same client to preserve context (DONE)
5) Server is connected to services and knows if a service goes down, and can adjust to it (WIP)
6) Command line interface for bringing up load balancer with settings, and updating load balancer configuration while it is running. This will require some basic threading (WIP)
7) Bidirectional UDP data: servers should be able to send UDP data back to the client they received the data from
8) Learn about ways to improve performance and implement them, perhaps by studying NGINX
9) Learn how to use concurrency library and see if this can be implemented (ideally multiple workers reading packets from a queue and doing work)
10) More algorithms and configuration settings

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