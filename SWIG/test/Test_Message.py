##
# @file
#
# Test the Supersocket Message object.
#
# @authors Benjamin Shanahan and David Brandman

from supersocket import Message, SetVerbose, ENABLE

if __name__ == "__main__":

    SetVerbose(ENABLE)

    m = Message()
    m._from = "Test"
    m.id = 1
    m.data = "EXAMPLE TEXT"

    n = Message()
    n._from = "Test"
    n.id = 1
    n.data = "EXAMPLE TEXT"

    print m, n
    

    areEqual = (m == n)


    notEqualTests = [False, False, False, False]

    n._from = "tes2"
    notEqualTests[0] = (m != n)
    n._from = m._from

    n.id = 2
    notEqualTests[1] = (m != n)
    n.id = m.id

    n.dlen = 0
    notEqualTests[2] = (m != n)
    n.dlen = m.dlen

    n._from = "EXAMPLE TES2"
    notEqualTests[3] = (m != n)
    n._from = m._from


    if all(notEqualTests) and areEqual:
        print "Test Passed!"
    else:
        print "Test Failed."