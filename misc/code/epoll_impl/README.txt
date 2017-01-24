epoll-implementation is a method to get data from network respecting
`SYNCHRONIZATION_POLICIES.txt`

This implementation uses epoll() to poll network socket for packet receiving. A
timeout would help in case where a packet miss and we want a Packet Loss
Concealment (look at epoll_transfer_diagram.pdf).


Advantages:
from epoll manual:
"[...] scales well to large numbers of watched file descriptors."
