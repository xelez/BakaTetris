import socket
import struct
import sys

import config

# Create the socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(config.server_address)

# Tell the operating system to add the socket to the multicast group
# on all interfaces.
group = socket.inet_aton(config.multicast_group)
mreq = struct.pack('4sL', group, socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

response_data = ";".join([ ":".join(addr) for addr in config.server_list ])

# Receive/respond loop
print >>sys.stderr, 'start/running on %s' % (config.server_address, )
while True:
    data, address = sock.recvfrom(1024)

    if config.debug_output:
        print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
        print >>sys.stderr, data

    sock.sendto(response_data, address)

