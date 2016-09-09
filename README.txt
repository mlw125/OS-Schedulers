Matthew Williams

I will try and go through how my program works. If I miss something here,
then that the code will have the right comments since I wrote them after making
sure everything works instead of at the end.

About the code: For my code I switched to vectors for my queues since I thought they
would be easier to move things around and I received fewer seg faults. For eventQueue,
position 0 is the event queue head. For readyQueue, position 0 is the one inside
the CPU. This made it easy to keep track of everything for me and stayed with me
throughout the many restarts I had. The vectors contain pointers to data since I thought
that would be faster to move then the structs, but it is likley uneeded. Generally it is
easier to destroy the pointers and create new ones then objects.
In general a process arrives and if it is the first process to arrive or the CPU is not busy, 
then a process will be created and a departure event created and place in the 
event queue where it needs to go. New arrivals are scheduled when a arrival 
event is processed. Processes will either have departure event
or have a time slice event depending on scheduler. 

FCFS - Process comes in, process leaves in the order it arrived.
HRRN - Process departs, check which process has the highest ratio. Then run that one
and repeat.
RR - Process arrives, schedule time slice, place at end of ready queue, schedule
next time slice. Repeat with a process leaving occasionally.
STRF - Process runs unless another shorter one arrives. Run shortest process next.

In general I would very much recommend not using me to write scheduler simulations 
unless of course everything works perfectly, but I have a feeling that is not the case.