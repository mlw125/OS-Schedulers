#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
using namespace std;

////////////////////////////////////////////////////////////////
// sample events
#define ARR 1 // arrival
#define DEP 2 // departure
#define T_Slice 3 // time slice
#define STRF 4 // shortest time remaining first
// .. add more events

//////////////////////////////////////////////////////////////////event structure
struct event{
	float time; // event time
	int type; // event type
	int p_id; // event id for corresponding process id
	float burst; // event service time
	struct event* next;
}; // end event
//////////////////////////////////////////////////////////////////process structure
struct process {
	float arrival; // process arrival time
	float start; // process start time
	float burst; // process service time
	float finish; // process finish time
	float left; // process remaining service time
	int p_id; // process id
}; // end process

////////////////////////////////////////////////////////////////
// function definition
void init();
void run_sim();
void generate_report();
float genexp(float);
// where the events of various types will be processed
void process_event_ARR();
void process_event_DEP();
void process_event_slice();
void process_event_STRF();
// where the events of various types will be scheduled
void schedule_event_ARR(event *);
void schedule_event_DEP();
void schedule_time_slice(float);
void schedule_event_STRF();
// where new process is created
void createProcess();
// where a process is stored
void storeInFinishQueue();
// used for STRF
void premption();

////////////////////////////////////////////////////////////////
//Global variables
vector<event *> eventQueue; // the event queue, a vector of event pointers
vector<process *> readyQueue; // the ready queue, a vector of process pointers
vector<process *> finishQueue; // the finish queue, a vector of process pointers
vector<int> queueSize; // vector to hold data about the ready queue size
float clk; // simulation clock
float mu; // easier than passing 1/serviceTime
int processes; // number of processes ompleted
int id; // ids of processes
int scheduleType; // scheduler type
float serviceTime; // the average service time
float lambda; // average arrival rate
float quantum; // quantum
bool serverBusy; // if the server is busy

////////////////////////////////////////////////////////////////
void init()
{
	// initialize all variables, states, and end conditions
	clk = 0;
	mu = (1 / serviceTime);
	processes = 0;
	serverBusy = false;
	id = 1;

    // first event scheduled
    event * first = new event;
    schedule_event_ARR(first);  // }
}
////////////////////////////////////////////////////////////////	//finishQueue.push_back(readyQueue[id]);
void generate_report()
{
    // get the total turnaround
	float turnaround = 0;
    for (int x = 0; x < finishQueue.size(); x++)
    {
        turnaround += finishQueue[x]->finish - finishQueue[x]->arrival;
    } // end for

    // sum the average processes in the ready queue at any given time
    int total = 0;
    for(int x = 0; x < queueSize.size(); x++)
    {
        total += queueSize[x];
    } // end for

    // only to display data for testing
	cout << lambda << " " << turnaround / (processes+1) << " ";
	cout << (processes+1) / clk << " ";
	cout << lambda * serviceTime << " " << total / queueSize.size() << endl;

    // will append the data to a CSV file to more easily get data
	fstream fout;
	fout.open("data.csv", ios::out | ios::app);
    fout << lambda << ";" << turnaround / (processes+1) << ";";
	fout << (processes+1) / clk << ";";
	fout << lambda * serviceTime << ";" << total / queueSize.size() << endl;
	fout.close();


}
////////////////////////////////////////////////////////////////
// returns a random number between 0 and 1
float urand()
{
	return((float)rand() / RAND_MAX);
}
/////////////////////////////////////////////////////////////
// returns a random number that follows an exp distribution
float genexp(float lambda2)
{
	float u, x;
	x = 0;
	while (x == 0)
	{
		u = urand();
		x = (-1 / lambda2)*log(u);
	}
	return(x);
}
////////////////////////////////////////////////////////////
void run_sim()
{
    // will run until process reaches the number, usually 10000
	while (processes < 10000)
	{
	    // set the clock to the current time
		clk = eventQueue[0]->time;
//cout << "Clock: " << clk << endl << endl;
        //create a new arrival
        if(eventQueue[0]->type == ARR)
        {
            event * temp = new event;
            schedule_event_ARR(temp);
        }
        //cout << "Looking: " << eventQueue[0]->p_id << " Event Type: " << eventQueue[0]->type << endl;
        // do something based on the event type
		switch (eventQueue[0]->type)
		{
            case ARR:
                //cout << "Process: " << eventQueue[0]->p_id << " arriving\n";
                process_event_ARR();
			break;
            case DEP:
                process_event_DEP();
                //cout << "Process: " << eventQueue[0]->p_id << " departed\n";
			break;
			case T_Slice:
                process_event_slice();
            break;
            default:
                cout << "\nInput data as follows: scheduler_type_(1-4) lambda service_time quantum" << endl;
			// error
			break;
		} // end switch

        // get the readyQueue size at this time
        queueSize.push_back(readyQueue.size());
        // delete the event, since it was dynamically allocated
        delete eventQueue[0];
        // erase the event head
        eventQueue.erase(eventQueue.begin());
	}
} // end run_sim()
////////////////////////////////////////////////////////////////

void process_event_ARR()
{
    // if the server is not busy:
    if(serverBusy == false)
    {
        // create the process that will be in the placed in the CPU
        createProcess();
        // schedule its departure for FCFS and HRRN since we know when it will leave
        if(scheduleType != 4)//(scheduleType == 1 || scheduleType == 3)
            schedule_event_DEP(); // end if
        // schedule a time slice for the event if RR
        else
            schedule_time_slice(clk + quantum); // end else if
        serverBusy = true; // server is now busy
    } // end if
    else
    {
        // if the server busy then create a new process and place it on the Ready Queue
        createProcess();
        // if the scheduler is STRF then check if the new arrival prempts the current running one.
        if(scheduleType == 2)
            premption(); // end if

    } // end else
} // end process_event_ARR()

void process_event_DEP()
{
    // if the readyQueue only has one thing in it: the process currently being processed by CPU
    if(readyQueue.size() == 1)
    {
        // set the server to false and store the process in the finish queue to access its data for the report,
        // finally delete the process pointer off of the ready queue.
        serverBusy = false;
        storeInFinishQueue();
        readyQueue.erase(readyQueue.begin());
    } // end if
    else
    {
        // if there are processes on the ready queue then store the process and schedule the departure of the
        // next event on the ready queue.
        storeInFinishQueue();
        readyQueue.erase(readyQueue.begin());
        schedule_event_DEP();
    } // end else
} // end proess_event_DEP()

//////////////////////////////////////////////////////////////// //schedules an event in the future
void schedule_event_ARR(event * eve)
{
        // set up the data for the arrival event
        eve->type = ARR;
        eve->time = genexp(lambda) + clk;
        eve->burst = genexp(mu);
        eve->p_id = id;
        id++;
//cout << "Arrival of " << eve->p_id << " Type: " << eve->type << endl;
        // if the event queue has nothing in it then add this event as the head
        if(eventQueue.size() == 0)
        {
            eventQueue.push_back(eve);
        } // end if
        else
        {
            // if the eventQueue only has one thing on it then add this event to the end.
            if(eventQueue.size() == 1)
            {
                eventQueue.push_back(eve);
            } // end if
            // otherwise find where in the queue the event should go based on its time of arrival.
            else
            {
                // find the position in the queue where the event will occur
                int pos = 0;
                for(int x = 1; x < eventQueue.size(); x++)
                {
                    if(eve->time < eventQueue[x]->time)
                    {
                        pos = x;
                        break;
                    } // end if
                } // end for
                // insert the event at the end.
                if(pos == 0)
                {
                    eventQueue.push_back(eve);
                    //cout << "Arrival:" << eve->p_id << endl;
                } // end if
                // insert the event at its correct time.
                else
                {
                    eventQueue.insert(eventQueue.begin() + pos, eve);
                } // end else
            } // end else
        } // end else
} // end schedule_event_ARR()

///////////////////////////////////////////////////////////////
void schedule_event_DEP()
{
    // set up the data for a departure event
	event * newDep = new event;
    newDep->burst = readyQueue[0]->burst;
    newDep->p_id = readyQueue[0]->p_id;
    newDep->type = DEP;

    // if the server is busy then we know the process  when will start as soon as it arrives
    if(serverBusy == false)
    {
        newDep->time = readyQueue[0]->arrival + readyQueue[0]->burst;
        readyQueue[0]->start = readyQueue[0]->arrival;
    } // end if
    // otherwise it will leave after the previous process  finishes
    else
    {
        // if the scheduler is STRF and the process hasn't been running before; it will have its start time now.
        // Then set the departure event's time to the finish time. This way is done since we cannot get the start
        // any where else easily.
        if(scheduleType == 2)
        {
            if(readyQueue[0]->left == readyQueue[0]->burst)
                readyQueue[0]->start = clk; // end if
            newDep->time = clk + readyQueue[0]->left; // end if
        } // end if
        // else get the start time and set the finish time
        else
        {
            newDep->time = clk + readyQueue[0]->burst;
            readyQueue[0]->start = clk;
        } // end else
    } // end else
    // if the event queue has nothing in it then add this event as the head
    if(eventQueue.size() == 0)
    {
        eventQueue.push_back(newDep);
    } // end if
    else
    {
        // if the eventQueue only has one thing on it then add this event to the end.
        if(eventQueue.size() == 1)
        {
            eventQueue.push_back(newDep);
        } // end if
        // other wise find where in the queue the event should go based on its departure time.
        else
        {
            // find the position in the queue where the event will occur
            int pos = 0;
            for(int x = 1; x < eventQueue.size(); x++)
            {
                if(newDep->time < eventQueue[x]->time)
                {
                    pos = x;
                    break;
                } // end if
            } // end for
            // insert the event at the end.
            if(pos == 0)
                eventQueue.push_back(newDep); // end if
            // insert the event at its correct time.
            else
                eventQueue.insert(eventQueue.begin() + pos, newDep); // end else
        } // end else
    } // end else
} // schedule_event_DEP()

/////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	scheduleType = atoi(argv[1]);
	lambda = atof(argv[2]);
	serviceTime = atof(argv[3]);
	quantum = atof(argv[4]);

	// parse arguments
	init();
	run_sim();
	generate_report();
	system("pause");
	return 0;
} // end main()

void createProcess()
{
    // create a new process
    process * newProcess = new process;

    // if the scheduler is FCFS
	if(scheduleType == 1)
    {
        // add it to the end of the ready queue
        readyQueue.push_back(newProcess);
        // initialize the data for the process, will be based off of the arrival event of the process
        // currently sitting at the event head.
        int rqSize = readyQueue.size();
        readyQueue[rqSize - 1]->arrival = eventQueue[0]->time;
        readyQueue[rqSize - 1]->burst = eventQueue[0]->burst;
        // sine we don't know when the process will start and stop at this current point
        readyQueue[rqSize - 1]->start = -1;
        readyQueue[rqSize - 1]->finish = -1;
        readyQueue[rqSize - 1]->p_id = eventQueue[0]->p_id;
    } // end if
    // if the scheduler is STRF
    else if(scheduleType == 2)
    {
        /// initialize the data
        readyQueue.push_back(newProcess);

        newProcess->arrival = eventQueue[0]->time;
        newProcess->burst = eventQueue[0]->burst;
        newProcess->start = -1;
        newProcess->finish = -1;
        // this will be how long the process has left for its service time
        newProcess->left = eventQueue[0]->burst;
        newProcess->p_id = eventQueue[0]->p_id;
    } // end else if
    // if the scheduler is HRRN
    else if(scheduleType == 3)
    {
        // add it to the end of the ready queue, we will place it where it needs to go every time another process departs.
        // but currently it is the newest arrival to the ready queue.
        readyQueue.push_back(newProcess);
        // initialize the data for the process, will be based off of the arrival event of the process
        // currently sitting at the event head.
        int rqSize = readyQueue.size();
        readyQueue[rqSize - 1]->arrival = eventQueue[0]->time;
        readyQueue[rqSize - 1]->burst = eventQueue[0]->burst;
        // sine we don't know when the process will start and stop at this current point
        readyQueue[rqSize - 1]->start = -1;
        readyQueue[rqSize - 1]->finish = -1;
        readyQueue[rqSize - 1]->p_id = eventQueue[0]->p_id;
    } // end else if
    // if the scheduler is RR
    else
    {
        // initialize the data
        newProcess->arrival = eventQueue[0]->time;
        newProcess->burst = eventQueue[0]->burst;
        newProcess->start = -1;
        newProcess->finish = -1;
        // this will be how long the process has left for its service time
        newProcess->left = eventQueue[0]->burst;
        newProcess->p_id = eventQueue[0]->p_id;

        // if there are zero of 1 processes on the ready queue (technically one, but position 0 is in the CPU)
        // then place it at the end to be processes next
        if(readyQueue.size() < 2)
            readyQueue.push_back(newProcess); // end if
        // otherwise put the process right behind the currently running one in readyQueue, to be given a time slice next.
        else
            readyQueue.insert(readyQueue.begin() + 1, newProcess); // end else
    } // end else
} // end createProcess()

void storeInFinishQueue()
{
    // if the scheduler is FCFS
    if(scheduleType == 1)
    {
        // we know when the process finished and thus can determine when it started
        readyQueue[0]->finish = clk;
        readyQueue[0]->start = clk - readyQueue[0]->burst;
        // place the pointer into the finish queue.
        finishQueue.push_back(readyQueue[0]);
        // increment the number of processes finished
        processes++;
    } // end if
    // if the scheduler is STRF
    else if(scheduleType == 2)
    {
        // we know when the process finished and thus can determine when it started
        readyQueue[0]->finish = clk;
        // place the pointer into the finish queue.
        finishQueue.push_back(readyQueue[0]);
        // increment the number of processes finished
        processes++;

    } //end else if
    // if the scheduler is HRRN
    else if(scheduleType == 3)
    {
        // we know when the process finished and thus can determine when it started
        readyQueue[0]->finish = clk;
        readyQueue[0]->start = clk - readyQueue[0]->burst;
        // place the pointer into the finish queue.
        finishQueue.push_back(readyQueue[0]);

        // if readyQueue is larger than one, then find the next process that should be ran
        if(readyQueue.size() > 1)
        {
            // search for the position of the process with the highest ratio
            int pos = 1;
            float wait = 0;
            float hRatio = 0;
            float highestRatio = 1;
            for(int x = 1; x < readyQueue.size(); x++)
            {
                wait = clk - readyQueue[x]->arrival;
                hRatio = (wait + readyQueue[x]->burst) / readyQueue[x]->burst;
                if(hRatio > highestRatio)
                {
                    pos = x;
                    highestRatio = hRatio;
                } // end if
            } // end for
            // create a temporary pointer to hold the process data, erase it from the ready queue,
            // and move to the beginning of the ready queue to be processed next.
            process * tempPtr = readyQueue[pos];
            readyQueue.erase(readyQueue.begin() + pos);
            readyQueue.insert(readyQueue.begin() + pos, tempPtr);
        } // end if
        // increment the number of processes finished
        processes++;
    } // end else if
    // if the scheduler is RR
    else
    {
        // get the finish time of the process and store the process pointer on the finish queue
        readyQueue[0]->finish = clk;
        finishQueue.push_back(readyQueue[0]);
        // increment the number of processes finished
        processes++;
    } // end else
} // end storeInFinishQueue()

// schedule departure here or not
void process_event_slice()
{
    // if readyQueue only has the currently running process in it
    if(readyQueue.size() == 1)
    {
        // if it leaves and nothing is there then save its data, remove it from the queue,
        // and say that the server is no longer busy.
        if(readyQueue[0]->left == 0)
        {
            // process leaves
            storeInFinishQueue();
            readyQueue.erase(readyQueue.begin());
            serverBusy = false;
        } // end if
        // if the process is not finished then move it to the back and schedule a new time slice event
        else
        {
            readyQueue.push_back(readyQueue[0]);
            readyQueue.erase(readyQueue.begin());
            // pass the time that the next time slice should finish
            schedule_time_slice(clk+quantum);
        } // end else
    } // end if
    // if readyQueue has more than process in it
    else
    {
        // if the process is ready to leave then store it, remove it, and schedule the next
        // processes time slice
        if(readyQueue[0]->left == 0)
        {
            // process leaves
            storeInFinishQueue();
            readyQueue.erase(readyQueue.begin());
            // pass the time that the next time slice should finish
            schedule_time_slice(clk+quantum);
        } // end if
        else
        {
            // move process to end of ready queue and schedule next time slice
            readyQueue.push_back(readyQueue[0]);
            readyQueue.erase(readyQueue.begin());
            // pass the time that the next time slice should finish
            schedule_time_slice(clk + quantum); // schedule_time_slice(clk)
        } // end else
    } // end else
} // end process_time_slice()

void schedule_time_slice(float sliceTime)
{
    // create a new time slice event and initialize the data
    event * slice = new event;
    slice->type = T_Slice;
    slice->burst = readyQueue[0]->burst;
    slice->p_id = readyQueue[0]->p_id;

    // if this is the processes first run then the start time is the this current time minus the quantum
    if(readyQueue[0]->burst == readyQueue[0]->left)
        readyQueue[0]->start = sliceTime - quantum; // end if

    // check if the process will finish before its time slice ends
    float remain = readyQueue[0]->left - quantum;
    // if so then set the time to the new time it will finish
    if(remain <= 0)
    {
        slice->time = sliceTime + readyQueue[0]->left;
        readyQueue[0]->left = 0;
    } // end if
    else
    {
        // otherwise the time it will finish is the time passed along.
        // also decrement the time left.
        slice->time = sliceTime;
        readyQueue[0]->left -= quantum;
    } // end else

    // check to see where the event will be place in the event queue
    // same as schedule_event_ARR and schedule_event_DEP
    if(eventQueue.size() == 0)
    {
        eventQueue.push_back(slice);
    } // end if
    else
    {
        // if the eventQueue only has one thing on it then add this event to the end.
        if(eventQueue.size() == 1)
        {
            eventQueue.push_back(slice);
        }
        // other wise find where in the queue the event should go.
        else
        {
            int pos = 0;
            for(int x = 1; x < eventQueue.size(); x++)
            {
                if(slice->time < eventQueue[x]->time)
                {
                    pos = x;
                    break;
                }
            }
            // insert the event at the end.
            if(pos == 0)
                eventQueue.push_back(slice);
            // insert the event at its correct time.
            else
                eventQueue.insert(eventQueue.begin() + pos, slice);
        } // end else
    } // end else
} // end schedule_time_slice(float)

void premption()
{
    // check the remaining time for the current running process
    float timeLeft = clk - readyQueue[0]->start;
    // get the last process in the queue aka the one that just arrived
    int readyEnd = readyQueue.size() - 1;
    // if the new arrival is a shorter job then the current one
    if(readyQueue[readyEnd]->left < timeLeft)
    {
        //cout << "Process will prempt" << endl;
        readyQueue[0]->left = timeLeft;

        // find where in the event queue the current departure event is
        int pos = 0;
        for(int x = 1; x < eventQueue.size(); x++)
        {
            if(eventQueue[x]->p_id == readyQueue[0]->p_id)
            {
                pos = x;
                break;
            } // end if
        } // end for

        // erase the departure event off of the queue
        eventQueue.erase(eventQueue.begin() + pos);

        // move the shorter process into the currently running position.
        process * temp = readyQueue[readyEnd];
        readyQueue.erase(readyQueue.begin() + readyEnd);
        readyQueue.insert(readyQueue.begin(), temp);
        temp = NULL;

        // schedule the departure of the new job
        schedule_event_DEP();
    }
    // if the new arrival will not prempt.
    else
    {
        // find where in the ready queue the new arrival should go, based on service time.
        int pos = 0;
        for(int x = 1; x < readyQueue.size(); x++)
        {
            if(readyQueue[x]->left > readyQueue[readyEnd]->left)
            {
                pos = x;
                break;
            } // end if
        } // end for

        // if the job is shorter than something else then move it to the new position,
        // otherwise it will stay at the end of the ready queue.
        if(pos != 0)
        {
            process * temp = readyQueue[readyEnd];
            readyQueue.erase(readyQueue.begin() + readyEnd);
            readyQueue.insert(readyQueue.begin() + pos, temp);
        } // end if
    }
}
