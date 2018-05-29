# Supersocket
Making sockets super in C

Supersocket was written to facilitate communication between processes that could be on the same or different computers. The idea is that rather than referring to a process by a known IP and port, you refer to it by a name. It's very lightweight, and has no external dependencies outside of the standard Linux libraries. 

```C
Supersocket alice 	= {0};
Supersocket bob     = {0};
InitializeSupersocket(&alice, "Alice", "127.0.0.1", 5000);
InitializeSupersocket(&bob, "Bob", "127.0.0.1", 5001);
```

The `bob` and 'alice' structures will create UDP and a TCP sockets at the indicated IP/ports. It'll also kindly setup a named AF_INET socket in case another process would like to communicate on the same computer. Supersocket uses connected sockets (for both AF_INET and AF_UNIX) for the slight speed increase, and when sending a message it will choose AF_INET if possibe. 

Suppose Alice would like to discover Bob, and then say hello. First, we launch threads that can respond to message requests.

```C
InitializeSupersocketListener(&alice);
InitializeSupersocketListener(&bob);
```

Next, we Discover Bob's contact information:

```C
char buffer[] = "Hi Bob, how are you doing?";
Message m = CreateMessage("Alice", 0, buffer, strlen(buffer));

int aliceToBob = DiscoverSupersocket(&alice, "Bob");
SendMessage(&alice, aliceToBob, &m);

```
If Bob wants to receive the message, he goes:

```C
Message r = CreateMessageBuffer(200);
ReceiveMessage(&bob, &r);
```

When Alice discovers Bob's contact information, "Bob" will be added to her contact list. Thereafter, whenever she calls 

```C
SendMessageToAll(&alice, &m);
```

Alice will send a message to Bob, and anyone else who will be in her contact list. 

There's a lot more functionality; this tool is very extensively documented.



## Building Python library

Supersocket has a [SWIG-generated](http://www.swig.org/) Python module. To build this module, check out the README in the 'SWIG/' directory of this repo.




Special thanks to [Ben Shanahan](https://github.com/benshanahan1 "Ben Shanahan's github") for creating the Display tool used for debugging! For the most up-to-date version of the Display tool, check out the [Display repository](https://github.com/benshanahan1/display "Display Library").