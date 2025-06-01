README
====

***Title: CLIENT SERVER APPLICATION***

**Author: Necula Mihail**

**Group: 323CAa**

**University year: 2024 - 2025**

---

Chapter 1 - Structure of a subscriber
====

<pre style="font-family: inherit; font-size: inherit; line-height: inherit; color: inherit; background: transparent; border: none">
A subscriber must have the ip and the port of the server to can connect
to it. After a connection is established, we have a socket through which
the communication takes place. The only case in which a connection can not
be created are:
    -> the server is down
    -> a subscriber with same id is already connected to server

After, we are interested just in 2 events (the receiving of a message from
stdin or server). The management of these 2 cases are:
    message from stdin -> subscriber -> server
    message from server -> subscriber -> stdout
So the subscriber is an intermediate which receives a message, process it and
send it further.

The protocol used to send messages from a subscriber to server has 2 fields:
    -> type (32 bits - a little to many bytes for our use)
    -> topic name (which uses MAX_TOPIC_SIZE + 1 bytes; + 1 for the NULL char)
The type can have 3 values:
    -> 0 => subscribe message
    -> 1 => unsubscribe message
    -> 2 => close the connection message
The topic field is used just for the first 2 types. Also, is mentionable that
these names can contains wildcards (* and +).

The protocol which is used by server to send messages to subscribers is
explained in the next chapter.
</pre>

---

Chapter 2 - Structure of the server
====

<pre style="font-family: inherit; font-size: inherit; line-height: inherit; color: inherit; background: transparent; border: none">
The server needs a valid port to can "speak" with the others. After, we
need 2 sockets. One through which can communicate with the UDP clients.
The other to can accept connections from the subscribers (the TCP clients).

The subscribers are saved using 3 data structures. Two hashmaps to can
access them in O(1) by their id or socket fd. Moreover, a list because we
will need to go through all of subscribers when we receive an UDP message.
The hashmaps are using efficient hash functions, which are taken from the
the DSA's lab (Data Structure and Algorithm's lab). The hashmaps uses chaining
to avoid the collisions. In plus, the load factor is 0,75. When this threshold
is reached, the number of lists from a map is doubled.

All these 3 structures uses the same pointer for every subscriber's info.
The server saves for every sub: his id, his socket fd and a trie with his
subscriptions.

We are interested just in 3 events (the receiving a message from stdin,
a udp client or a subscriber). The management of these 3 cases are:
    message from stdin -> server -> all subscribers
    message from udp client -> server -> some subscribers
    message from subscriber -> server -> at subscriber back, sometimes

The protocol which is used by subscriber to send messages to server was
explained in the previous chapter. The protocol of the udp clients to send
messages to server will be talked in the next section.

Now, we focus on the protocol made by server to discuss with the tcp clients.
This one has 5 fields:
    -> type (8 bits)
    -> udp data's provider ip (32 bits)
    -> udp data's provider port (16 bits)
    -> topic name (which uses MAX_TOPIC_SIZE + 1 bytes; + 1 for the NULL char)
    -> data / payload (which uses MAX_DATA_SIZE + 1 bytes)
The type can have 5 values:
    -> 0-3 => type of the received messages from udp
    -> 4 => close the connection message
The other fields are used just in the first 4 types, when we send further a message
from an udp to a subscriber.
</pre>

---

Chapter 3 - Structure of the udp client
====

<pre style="font-family: inherit; font-size: inherit; line-height: inherit; color: inherit; background: transparent; border: none">
The udp clients just send messages to server. The protocol for this contains
the next fields:
    -> type (8 bits)
    -> topic name (which uses MAX_TOPIC_SIZE bytes)
    -> data (which uses MAX_DATA_SIZE bytes)
The type can have 4 values, from 0 to 3. This codifies the type of data which
is send. It's useful to subscribers to process the message. The topic and data
are not NULL terminated if their size are equal with the maximum one.
</pre>
