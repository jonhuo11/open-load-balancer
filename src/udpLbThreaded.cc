

// concurrent safe control block
// mapping clients to services and reverse
struct ControlBlock {

};

// work queue: data is read off the socket and placed in the queue here

// thread pool: takes data from work queue, uses control block to concurrently perform load balancing
void workerThread() {

}


void driver() {

}