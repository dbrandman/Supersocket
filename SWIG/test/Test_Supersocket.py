##
# @file
#
# Test the Supersocket object and related functions.
#
# @authors Benjamin Shanahan and David Brandman

from supersocket import *

if __name__ == "__main__":

    SetVerbose(ENABLE)

    sSend    = Supersocket()
    sReceive = Supersocket()

    InitializeSupersocket(sReceive, "UnitTest", "127.0.0.1", 5000)

    AddSocket(sReceive, "UnitTest1", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, BIND)
    AddSocket(sReceive, "UnitTest2", "127.0.0.1", 5002, AF_UNIX, SOCK_DGRAM, BIND)
    AddSocket(sReceive, "UnitTest2", "239.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND | MULTICAST)

    AddSocket(sSend, "UnitTest", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT)
    AddSocket(sSend, "UnitTest", "239.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT)
    AddSocket(sSend, "UnitTest", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, CONNECT)
    AddSocket(sSend, "UnitTest", "127.0.0.1", 5001, AF_UNIX, SOCK_DGRAM, CONNECT)

    m = Message()
    m.data = "My Message!"

    r = Message()

    # print sReceive, sSend

    SendMessageToAll(sSend, m)

    print "Waiting for Message 1..."
    ReceiveMessage(sReceive, r)
    print r

    print "Waiting for Message 2..."
    ReceiveMessage(sReceive, r)
    print r

    print "Waiting for Message 3..."
    ReceiveMessage(sReceive, r)
    print r

    print "Waiting for Message 4..."
    ReceiveMessage(sReceive, r)
    print r