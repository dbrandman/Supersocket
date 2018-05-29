from supersocket import *

if __name__ == "__main__":

    SetVerbose(ENABLE)

    alice = Supersocket()
    bob   = Supersocket()

    aliceToBob = AddSocket(alice, "Bob", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, CONNECT)
    AddSocket(bob, "Bob", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, BIND)

    m = Message()
    m.data = "My message!"

    r = Message()
    r.initialize(20)

    SendMessage(alice, aliceToBob, m)
    print "Sent:"
    print m, m.data

    ReceiveMessage(bob, r)
    print "Received:"
    print r, r.data