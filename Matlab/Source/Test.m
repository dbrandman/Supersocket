alice = CreateSupersocket()
bob   = CreateSupersocket()

InitializeSupersocket(alice,'Alice','127.0.0.1',5000)
InitializeSupersocket(bob,'Bob','127.0.0.1',6000)

InitializeSupersocketListener(alice)
InitializeSupersocketListener(bob)

%%

DiscoverSocket(alice, 'Bob');

%%

stringToSend = uint8('Hello Bob! How are you?');
SendData(alice, stringToSend, length(stringToSend));
fprintf('[Alice]: %s\n',ReceiveData(bob));


