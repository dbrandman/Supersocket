##
# @file
#
# Test the Supersocket Listener function
#
# @authors Benjamin Shanahan and David Brandman

from supersocket import                                                 \
    SetVerbose, SetFilename, ENABLE, DISABLE,                           \
    DiscoverSocket,                                                     \
    Supersocket, InitializeSupersocket, InitializeSupersocketListener,  \
    AF_INET, AF_UNIX, SOCK_DGRAM, BIND, CONNECT, MULTICAST,             \
    Message, SendMessage, ReceiveMessage


if __name__ == "__main__":

    SetVerbose(ENABLE)
    SetFilename("Test_SupersocketListener")

    alice = Supersocket()
    bob   = Supersocket()

    InitializeSupersocket(alice, "Alice", "127.0.0.1", 5000)
    InitializeSupersocket(bob, "Bob", "127.0.0.1", 5001)

    InitializeSupersocketListener(alice)
    InitializeSupersocketListener(bob)

    # raw_input("Push ENTER")

    DiscoverSocket(alice, "Bob")  # Tell Alice to find Bob
    DiscoverSocket(bob, "Alice")  # Tell Bob to find Alice

    # raw_input("Push ENTER")

    m = Message()
    m._from = "Alice"
    m.id = 4
    m.data = "THIS IS SOME TEXT!"
    SendMessage(alice, m)

    r = Message()
    r.initialize(200)

    ReceiveMessage(bob, r)
    print r