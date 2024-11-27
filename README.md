# Open Load Balancer

A load balancer written in C++ to learn more about networking, Linux sockets, and C++ for performance.

# Development stages

1) Server can read incoming byte data on a UDP socket (DONE)
2) Server can write out to 1 client on a UDP socket (DONE)
3) Server can write to multiple clients in round robin (DONE)
4) Server can take incoming IP address from header and route same IP to same client to preserve context (WIP)
5) Server is connected to services and knows if a service goes down, and can adjust to it
6) Command line interface for bringing up load balancer with settings, and updating load balancer configuration while it is running
8) Learn about ways to improve performance and implement them, perhaps by studying NGINX
9) Learn how to use pthreads and see if this can be implemented
10) More algorithms and configuration settings
