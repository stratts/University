# Individual Assignment
# Circular DHT Peer to Peer
#
# by Stratton Sloane
# 29-04-2019


import sys
import socket
import time
import re
import threading
import queue
import enum
import struct
import random

# -- Classes --

class MessageType(enum.IntEnum):
    PING_REQ = 1 
    PING_RESP = 2
    FILE_REQUEST = 3
    HAS_FILE = 4
    PACKET_TRN = 5
    PACKET_RCV = 6
    DEPART = 7
    SUCCESSORS_REQ = 8
    SUCCESSORS_RESP = 9


# The Message class provides a format for transferring information
# between peers, and provides functions to convert to a byte
# format suitable for transmission
class Message:
    def __init__(self, message_type = 0, data = None):
        self.m_type = int(message_type)
        self.source = 0             # Source peer no. (ie, self)
        self.dest = 0               # Destination peer no.
        self.successors = b''       # Successors of peer
        self.requested_peer = 0     # Peer requesting file
        self.requested_file = 0     # Requested file name
        self.seq_number = 0
        self.mss = 0
        self.segment_size = 0
        self.file_data = b''

        self.header_format = 'BBB2sBHII'

        if data: self.unpack(data)

    # Returns the message as a string of bytes
    def pack(self):
        struct_format = self.header_format + '{}s'.format(self.segment_size)
        return struct.pack(struct_format, 
            self.m_type, 
            self.source, 
            self.dest, 
            self.successors,
            self.requested_peer, 
            self.requested_file,
            self.seq_number,
            self.segment_size,
            self.file_data
        )

    # Unpacks the message from a string of bytes
    def unpack(self, data):
        header_size = struct.calcsize(self.header_format)
        (self.m_type, 
        self.source, 
        self.dest,
        self.successors,
        self.requested_peer,
        self.requested_file,
        self.seq_number,
        self.segment_size) = struct.unpack(self.header_format, data[:header_size])

        self.file_data = struct.unpack('{}s'.format(self.segment_size), data[header_size:])[0]


# The FileManager class is responsible for both requesting and handling
# requests, and both transferring and receiving files
class FileManager:
    def __init__(self, listener, sender, drop_chance):
        self.listener = listener
        self.sender = sender
        self.drop_chance = drop_chance
        self.log_format = '{:20s}{:16.1f}{:8d}{:8d}{:8d}\n'
        pass

    def request_file(self, filename):
        message = Message(MessageType.FILE_REQUEST)
        message.requested_file = filename
        message.requested_peer = self.sender.identity
        self.sender.forward_message(message, True)

        response = listener.get_message(MessageType.HAS_FILE)
        self.get_file(response)
    
    # Determines whether to transfer file or forward request to successor
    def serve_request(self, message):
        filename = message.requested_file
        hash = filename % 256
        identity = self.sender.identity

        if (hash == identity or (hash < identity and message.source < hash) or
           (hash > identity and message.source > identity and message.source < hash)):
            print('File {:04d} is here.'.format(filename))
            self.send_file(message.requested_peer, filename)
        else:
            print('File {:04d} is not stored here.'.format(filename))
            self.sender.forward_message(message, True)
            print('File request message has been sent to my successor.')

    # Transfers the given filename to a peer
    def send_file(self, peer, filename):

        # Send response message, to let peer know we have the file
        initial_message = Message(MessageType.HAS_FILE)
        initial_message.requested_file = filename
        initial_message.seq_number = 0

        self.sender.send_message(peer, initial_message, True)
        print('A response message, destined for peer {}, has been sent.'.format(peer))
        print('We now start sending the file...')

        mss = self.sender.mss
        seq_num = 1
        retransmit = False

        log = open('responding_log.txt', 'w') 

        with open('{:04d}.pdf'.format(filename), 'rb') as f:       
            buffer = f.read(mss) # Read MSS bytes from buffer

            while(buffer):
                # Create message containing segment
                message = Message(MessageType.PACKET_TRN)
                message.requested_file = filename
                message.seq_number = seq_num
                message.segment_size = len(buffer)
                message.file_data = buffer

                # Select category for log, and determine whether to drop
                cat = 'snd'

                if not random.random() < self.drop_chance:
                    self.sender.send_message(peer, message)
                else: cat = 'Drop'

                if retransmit: 
                    if cat == 'snd': cat = 'RTX'
                    else: cat = 'Drop/RTX'
                
                log.write(self.log_format.format(
                    cat, time.time(), seq_num, message.segment_size, 0)
                )

                retransmit = False

                # Wait for ACK response from receiver
                recv = listener.get_message(MessageType.PACKET_RCV, timeout=1)

                if recv:
                    log.write(self.log_format.format(
                        'recv', time.time(), 0, recv.segment_size, recv.seq_number)
                    )
                    # Read next segment into buffer
                    buffer = f.read(mss)
                    seq_num += mss
                else: retransmit = True
                
            # Send message with sequence number of 0 to indicate transfer completion
            message = Message(MessageType.PACKET_TRN)
            message.requested_file = filename
            message.seq_number = 0
            self.sender.send_message(peer, message)

            print('The file is sent')
        
        log.close()

    # Takes a HAS_FILE message and starts saving packets to file
    def get_file(self, message):
        print('Received a response message from peer {}, which has the file {:04d}.'.format(
            message.source, message.requested_file
        ))

        seq_num = 1
        log = open('requesting_log.txt', 'w')

        with open('received_file.pdf', 'wb') as f:
            print('We now start receiving the file...')

            while True:
                recv = listener.get_message(MessageType.PACKET_TRN)

                # Seq number of 0 is final packet, so exit
                if (recv.seq_number == 0):
                    print('The file is received')
                    break

                log.write(self.log_format.format(
                    'recv', time.time(), recv.seq_number, recv.segment_size, 0)
                )

                # Write to file if not duplicate
                if (recv.seq_number == seq_num):
                    f.write(recv.file_data)
                    seq_num += recv.segment_size
                
                # Generate and send ACK response
                resp = Message(MessageType.PACKET_RCV)
                resp.seq_number = seq_num
                resp.requested_file = recv.requested_file
                resp.segment_size = recv.segment_size

                sender.send_message(recv.source, resp)

                log.write(self.log_format.format(
                    'snd', time.time(), 0, recv.segment_size, seq_num)
                )

        log.close()


# The PeerManager class is responsible for pinging peers, 
# checking whether they are alive, and replacing peers when they drop out
# It also handles requests from other peers relating to those functions
class PeerManager:
    def __init__(self, successors, listener, sender):
        self.successors = successors
        self.predecessors = set()
        self.alive = set()
        self.listener = listener
        self.sender = sender

        self.ping_attempts = 3
        self.ping_timeout = 1

        self.exit = False

        listener.add_handler(MessageType.PING_REQ, self.ping_response)

    def ping_loop(self, interval):
        while not self.exit:
            self.ping_successors()
            time.sleep(interval)

    def ping_successors(self):

        self.alive.clear()
        start = time.time()
        attempts = 0

        while len(self.alive) != len(self.successors) and attempts < self.ping_attempts:
            for successor in self.successors:
                if successor in self.alive: continue
                self.sender.send_message(successor, Message(MessageType.PING_REQ))

            while time.time() - start < self.ping_timeout:
                response = listener.get_message(MessageType.PING_RESP, False)

                if response:
                    peer = response.source
                    print('A ping response message was received from Peer {}.'.format(peer))
                    self.alive.add(peer)

                time.sleep(0.05)

            attempts += 1
            start = time.time()

        for successor in self.successors:
            if successor not in self.alive:
                alive_peer = list(self.alive)[0]
                print('Peer {} is no longer alive.'.format(successor))

                if self.successors.index(alive_peer) == 0:
                    s1 = self.get_successors(alive_peer)[1]
                    s2 = self.get_successors(s1)[0]
                else:
                    s1 = self.successors[1]
                    s2 = self.get_successors(alive_peer)[0]

                self.replace_peer(successor, [s1, s2])

    def ping_response(self, message):
        peer = message.source
        print('A ping request message was received from Peer {}.'.format(peer))
        m = Message(MessageType.PING_RESP)
        m.successors = bytes(self.successors)
        self.sender.send_message(peer, m)

        if len(self.predecessors) == 2: self.predecessors.clear()
        self.predecessors.add(peer)

    def get_successors(self, peer):
        self.sender.send_message(peer, Message(MessageType.SUCCESSORS_REQ))
        m = self.listener.get_message(MessageType.SUCCESSORS_RESP, timeout=5)
        return m.successors

    def successors_response(self, message):
        m = Message(MessageType.SUCCESSORS_RESP)
        m.successors = bytes(self.successors)
        self.sender.send_message(message.source, m, True)

    def depart(self):
        for p in self.predecessors:
            m = Message(MessageType.DEPART)
            m.successors = bytes(self.successors)
            self.sender.send_message(p, m, True)

    def replace_peer(self, peer, successors):
        if peer == self.successors[0]:
            self.successors[0] = self.successors[1]
            self.successors[1] = successors[1]
        else:
            self.successors[1] = successors[0]

        print('My first successor is now peer {}.'.format(self.successors[0]))
        print('My second successor is now peer {}.'.format(self.successors[1]))

        self.sender.successor = self.successors[0]
        self.ping_successors()

    def peer_departed(self, message):
        print('Peer {} will depart from the network'.format(message.source))
        self.replace_peer(message.source, message.successors)

    def quit(self): self.exit = True
        

# The MessageSender class sends messages to peers over UDP and TCP
class MessageSender:
    def __init__(self, identity, successor, mss):
        self.identity = identity
        self.successor = successor
        self.mss = mss

    # Sends message to given peer
    def send_message(self, peer, message, tcp = False):
        message.dest = peer
        self._send(message, tcp)

    # Forwards message to first successsor 
    def forward_message(self, message, tcp = False):
        message.dest = self.successor
        self._send(message, tcp)

    def _send(self, message, tcp):  
        message.source = self.identity
        message.mss = self.mss
        address = ('127.0.0.1', 50000 + message.dest)

        if tcp:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(address)
            sock.send(message.pack())
            sock.close()
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.setblocking(False)
            sock.sendto(message.pack(), address)

# The Listener class listens for messages over both UDP and TCP, processes
# and stores received messages, and calls the functions to handle them
class Listener:
    def __init__(self, port, mss):
        self.messages = { m:queue.Queue() for m in MessageType }
        self.port = port
        self.mss = mss

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', port))
        self.sock.settimeout(1)

        self.sock_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock_tcp.bind(('', port))
        self.sock_tcp.listen(1)
        self.sock_tcp.settimeout(1)

        self.buffer_size = 1024
        self.exit = False

        self.handlers = {}

    def listen(self):
        while not self.exit:
            try:
                data = self.sock.recv(self.buffer_size)
                self.process_data(data)
            except socket.timeout: pass

        self.sock.close()

    def listen_tcp(self):
        while not self.exit:
            try:
                conn, _ = self.sock_tcp.accept()
                data = conn.recv(self.buffer_size)
                self.process_data(data)
                conn.close()
            except socket.timeout: pass

        self.sock_tcp.close()

    # Converts received data to Message object and calls handlers
    # for that message type
    def process_data(self, data):
        message = Message()
        message.unpack(data)

        if message.m_type in self.handlers:
            for handler in self.handlers[message.m_type]:
                t = threading.Thread(target=handler, args=[message])
                t.start()

        self.messages[message.m_type].put(message)

    def get_message(self, type, block = True, timeout = None):
        try: item = self.messages[type].get(block, timeout)
        except queue.Empty: return None
        return item

    def add_handler(self, message_type, function):
        if not message_type in self.handlers:
            self.handlers[message_type] = set()

        self.handlers[message_type].add(function)

    def quit(self):
        self.exit = True


# -- Main program --


# Read command line arguments
identity = int(sys.argv[1])
successor1 = int(sys.argv[2])
successor2 = int(sys.argv[3])
mss = int(sys.argv[4])
drop_chance = float(sys.argv[5])

# Set up message listener and sender
listener = Listener(50000 + identity, mss)
sender = MessageSender(identity, successor1, mss)

# Create peer manager and listen for peer messages
peer_manager = PeerManager([successor1, successor2], listener, sender)
listener.add_handler(MessageType.DEPART, peer_manager.peer_departed)
listener.add_handler(MessageType.SUCCESSORS_REQ, peer_manager.successors_response)

# Create file manager and listen to file requests
file_manager = FileManager(listener, sender, drop_chance)
listener.add_handler(MessageType.FILE_REQUEST, file_manager.serve_request)

# Start UDP and TCP listen threads
listen_thread = threading.Thread(target=listener.listen)
listen_thread.start()
listen_thread_tcp = threading.Thread(target=listener.listen_tcp)
listen_thread_tcp.start()

# Wait for other peers to come online
time.sleep(2)

# Start pinging
ping_thread = threading.Thread(target=peer_manager.ping_loop, args=[10])
ping_thread.start()

# Wait for and process terminal input
while True:
    command = input()
    if re.match('request [0-9]{4}', command):
        filename = command.split(' ')[1]
        file_manager.request_file(int(filename))
    elif command == 'quit':
        peer_manager.quit()
        peer_manager.depart()
        listener.quit()
        sys.exit()
    else: print("Invalid command. Commands are 'request <filename>' or 'quit'")
