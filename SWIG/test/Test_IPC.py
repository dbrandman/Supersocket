##
# @file
#
# Test Supersocket communication between Python processes.
# 
# These processes are very simple and are meant to illustrate interprocess 
# communication between independent Python processes using the C Supersocket 
# library.
# 
# This script takes two command line arguments. The first argument specifies 
# the name of the Supersocket for the current Python process, and the second 
# argument specifies the name of the Supersocket for the process with which it
# will try to communicate (send and receive messages).
# 
# To use this test, launch two separate processes, for example:
# 
#     In terminal ONE:
#         $ python -i test_Python_IPC.py Alice Bob
# 
#     In terminal TWO:
#         $ python -i test_Python_IPC.py Bob Alice
# 
# @author Benjamin Shanahan

import sys
import supersocket

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print """Run this program in two terminals (separate processes).
        Usage:
            First terminal:
                $ python Test_IPC Alice Bob
            Second terminal:
                $ python Test_IPC Bob Alice
        """
        exit()

    # Manually configure Display parameters
    supersocket.SetVerbose(supersocket.DISABLE);
    supersocket.SetFilename(__file__[:-3])

    # Parse command line arguments
    THIS = sys.argv[1]  # This process
    THAT = sys.argv[2]  # That other process we talk to

    # Create a new Supersocket and find the other one
    thisSupersocket = supersocket.Supersocket()
    supersocket.InitializeSupersocket(thisSupersocket, THIS, "127.0.0.1", 0)
    supersocket.InitializeSupersocketListener(thisSupersocket)

    destinationID = supersocket.DiscoverSupersocket(thisSupersocket, THAT)
    print "Found other process!"

    # Create a message to send
    m = supersocket.Message()
    m._from = THIS
    m.data  = "Hello %s, I am %s!" % (THAT, THIS)

    # Allocate a message to receive into
    r = supersocket.Message()
    r.initialize(50)  # allocate and zero memory

    # Send and receive!
    supersocket.SendMessage(thisSupersocket, destinationID, m)
    supersocket.ReceiveMessage(thisSupersocket, r)

    print "I sent a", m, m.data
    print ""
    print "I got a", r, r.data

    # Clean up
    supersocket.CloseSupersocket(thisSupersocket)

    # raw_input("Press ENTER to continue")