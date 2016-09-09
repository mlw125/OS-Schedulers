#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////////
//.___           ,    __. ,           ,
//[__ .  , _ ._ -+-  (__ -+-._.. . _.-+-. .._. _
//[___ \/ (/,[ ) |   .__) | [  (_|(_. | (_|[  (/,
////////////////////////////////////////////////////////////////
struct Event{
    int type;
    float arr_t, serv_t, rem_t, fin_time, r_ratio;

    Event(float init_arr_t, float init_serv_t){
        arr_t = init_arr_t;
        serv_t = init_serv_t;
        rem_t = init_serv_t;
    }
};
////////////////////////////////////////////////////////////////
//.___          ,          .__    ._
//[__ . .._  _.-+-* _ ._   |  \ _ |,
//|   (_|[ )(_. | |(_)[ )  |__/(/,|  *
////////////////////////////////////////////////////////////////
void init(float avg_arr_t, float avg_serv_t);
int fcfs_sim(float avg_arr_t, float avg_serv_t);
void generate_report();
float genexp(float lambda);
////////////////////////////////////////////////////////////////
//.__ .   .     .  .  .
//[ __| _ |_  _.|  \  / _.._.
//[_./|(_)[_)(_]|   \/ (_][   *
////////////////////////////////////////////////////////////////
vector <Event> event_queue;
vector <Event> finish_queue;
vector <Event> holding_queue;
float p_clock, finish_time;
float num_ready_processes;
////////////////////////////////////////////////////////////////
//.___          ,          ._.
//[__ . .._  _.-+-* _ ._    | ._ _ ._
//|   (_|[ )(_. | |(_)[ )  _|_[ | )[_) *
//                                 |
////////////////////////////////////////////////////////////////
//initialize all variables, states, and end conditions
////////////////////////////////////////////////////////////////
void init(float avg_arr_t, float avg_serv_t){
    p_clock = 0.0;
    finish_time = 0.0;
    num_ready_processes = 0.0;
	Event event(0, genexp(avg_serv_t));
	event_queue.push_back(event);
	holding_queue.push_back(event);
}
////////////////////////////////////////////////////////////////
//Prints out stats derived from simulation
////////////////////////////////////////////////////////////////
void generate_report(){
    float turnaround_ttl;
    for(int i = 0; i < 10000; i++){
        turnaround_ttl += (finish_queue[i].fin_time - finish_queue[i].arr_t);
    }
    cout << endl;
	cout << "Average Turnaround                : " << turnaround_ttl/10000 << endl;
	cout << "Total Throughput                  : " << 10000/p_clock << endl;
	cout << "CPU Utilization                   : " << finish_queue.back().fin_time/p_clock << endl;
	cout << "Average # Processes in Ready Queue: " << num_ready_processes/p_clock << endl;
}

////////////////////////////////////////////////////////////////
// returns a random number between 0 and 1
////////////////////////////////////////////////////////////////
float urand(){
    float ran_num = 0;
    while(ran_num == 0){
        ran_num = ((float) rand()/RAND_MAX );
    }
    return(ran_num);
}

////////////////////////////////////////////////////////////////
// returns a random number that follows an exp distribution
////////////////////////////////////////////////////////////////
float genexp(float lambda){
	float u,x;
	x = 0;
	while (x == 0){
        u = urand();
        x = (-1/lambda)*log(u);
    }
	return(x);
}

void printVector(vector<Event>& A) {
  for (unsigned int i=0; i<A.size(); i++) {
    cout << A[i].r_ratio << " ";
  }
  cout<< endl;
  cout << "============================================" << endl;
}
////////////////////////////////////////////////////////////////
//.___ __ .___ __.
//[__ /  `[__ (__
//|   \__.|   .__)
////////////////////////////////////////////////////////////////
//First come first serve simulation
////////////////////////////////////////////////////////////////
int fcfs_sim(float avg_arr_t, float avg_serv_t){
    while (finish_queue.size() < 10000){
        float arr_gap = genexp(avg_arr_t);
        float arr_t = event_queue.back().arr_t + arr_gap;
        float serv_t = genexp(avg_serv_t);

        Event event(arr_t, serv_t);
        num_ready_processes += event_queue.size();

        p_clock = p_clock + arr_gap;
        finish_time = finish_time + arr_gap;

        event_queue.push_back(event);

        if((event_queue.front().rem_t - arr_gap) <= 0){
            if((event_queue.front().rem_t - arr_gap) < 0){
                finish_time = finish_time - (arr_gap - event_queue.front().rem_t);
            }
            event_queue.front().fin_time = finish_time;
            finish_queue.push_back(event_queue.front());
            event_queue.erase(event_queue.begin());
        } else{
            event_queue.front().rem_t -= arr_gap;
        }
        //cout << "\r" << finish_queue.size() << " processes completed.       " << flush;
    }
    return 0;
}
////////////////////////////////////////////////////////////////
// __..__ .___..___
//(__ [__)  |  [__
//.__)|  \  |  |
////////////////////////////////////////////////////////////////
//Sort queue in ascending order based on remaining time
////////////////////////////////////////////////////////////////
void rem_insertion_sort (vector<Event>& A, int length){
    int j;
    for (int i = 0; i < length; i++){
		j = i;
        while (j > 0 && A[j].rem_t < A[j-1].rem_t){
            swap(A[j], A[j-1]);
			j--;
        }
    }
}
void srtf_sort(){
    rem_insertion_sort(event_queue, event_queue.size());
}
////////////////////////////////////////////////////////////////
//Shortest remaining time simulation
////////////////////////////////////////////////////////////////
int srtf_sim(float avg_arr_t, float avg_serv_t){
    while (finish_queue.size() < 10000){
        float arr_gap = genexp(avg_arr_t);
        float arr_t = event_queue.back().arr_t + arr_gap;
        float serv_t = genexp(avg_serv_t);

        Event event(arr_t, serv_t);
        num_ready_processes += event_queue.size();

        p_clock = p_clock + arr_gap;
        finish_time = finish_time + arr_gap;

        event_queue.push_back(event);
        if((event_queue.front().rem_t - arr_gap) <= 0){
            if((event_queue.front().rem_t - arr_gap) < 0){
                finish_time = finish_time - (arr_gap - event_queue.front().rem_t);
            }
            event_queue.front().fin_time = finish_time;
            finish_queue.push_back(event_queue.front());
            event_queue.erase(event_queue.begin());
        } else{
            event_queue.front().rem_t -= arr_gap;
        }
        srtf_sort();

        //cout << "\r" << finish_queue.size() << " processes completed.       " << flush;
    }
    return 0;
}
////////////////////////////////////////////////////////////////
//.  ..__ .__ .  .
//|__|[__)[__)|\ |
//|  ||  \|  \| \|
////////////////////////////////////////////////////////////////
//Sort queue in ascending order based on response ratio
////////////////////////////////////////////////////////////////
void ratio_insertion_sort (vector<Event>& A, int length){
    int j;
    for (int i = 0; i < length; i++){
		j = i;
        while (j > 0 && A[j].r_ratio > A[j-1].r_ratio){
            swap(A[j], A[j-1]);
			j--;
        }
    }
}
void hrrn_sort(){
    ratio_insertion_sort(event_queue, event_queue.size());
}
////////////////////////////////////////////////////////////////
//Calculate response ratios for each process
////////////////////////////////////////////////////////////////
void calculate_ratio(vector<Event>& A){
    for(unsigned int i = 0; i < A.size(); i++){
        A[i].r_ratio = ((finish_time - A[i].arr_t) + A[i].serv_t)/A[i].serv_t;
    }
}
////////////////////////////////////////////////////////////////
//Highest response ratio next simulation
////////////////////////////////////////////////////////////////
int hrrn_sim(float avg_arr_t, float avg_serv_t){
    while (finish_queue.size() < 10000){
        float arr_gap = genexp(avg_arr_t);
        float arr_t = event_queue.back().arr_t + arr_gap;
        float serv_t = genexp(avg_serv_t);

        Event event(arr_t, serv_t);
        num_ready_processes += event_queue.size();

        p_clock = p_clock + arr_gap;
        finish_time = finish_time + arr_gap;

        event_queue.push_back(event);
        if((event_queue.front().rem_t - arr_gap) <= 0){
            if((event_queue.front().rem_t - arr_gap) < 0){
                finish_time = finish_time - (arr_gap - event_queue.front().rem_t);
            }
            event_queue.front().fin_time = finish_time;
            finish_queue.push_back(event_queue.front());
            event_queue.erase(event_queue.begin());
            calculate_ratio(event_queue);
            hrrn_sort();
        } else{
            event_queue.front().rem_t -= arr_gap;
        }

        //cout << "\r" << finish_queue.size() << " processes completed.       " << flush;
    }
    return 0;
}
////////////////////////////////////////////////////////////////
//.__            .  .__    .
//[__) _ . .._  _|  [__) _ |_ *._
//|  \(_)(_|[ )(_]  |  \(_)[_)|[ )
////////////////////////////////////////////////////////////////
int rr_sim(float avg_arr_t, float avg_serv_t, float q_bit){
    while (finish_queue.size() < 10000){
        float arr_gap = genexp(avg_arr_t);
        float arr_t = holding_queue.back().arr_t + arr_gap;
        float serv_t = genexp(avg_serv_t);

        Event event(arr_t, serv_t);
        num_ready_processes += event_queue.size();

        p_clock = p_clock + q_bit;
        finish_time = finish_time + q_bit;

        holding_queue.push_back(event);

        if(holding_queue.at(1).arr_t <= p_clock){
            event_queue.insert(event_queue.begin(), holding_queue.at(1));
            holding_queue.erase(holding_queue.begin()+1);
        }

        if((event_queue.front().rem_t - q_bit) <= 0){
            if((event_queue.front().rem_t - q_bit) < 0){
                finish_time = finish_time - (q_bit - event_queue.front().rem_t);
            }
            event_queue.front().fin_time = finish_time;
            finish_queue.push_back(event_queue.front());
            event_queue.erase(event_queue.begin());
        } else{
            event_queue.front().rem_t -= q_bit;
            event_queue.push_back(event_queue.front());
            event_queue.erase(event_queue.begin());
        }

        //cout << "\r" << finish_queue.size() << " processes completed.       " << flush;
    }
    return 0;
}
////////////////////////////////////////////////////////////////
//.  ..__.._..  .
//|\/|[__] | |\ |
//|  ||  |_|_| \|
////////////////////////////////////////////////////////////////
int main(int argc, char *argv[] ){
    // parse arguments
    int simulation = atoi(argv[1]);
    float avg_arr_t = atof(argv[2]);
    float avg_serv_t = atof(argv[3]);
    float q_bit = atof(argv[4]);

    //Run simulation if all arguments are included
    if(argc == 5){
        init(avg_arr_t, avg_serv_t);
        switch(simulation){
            case 1: fcfs_sim(avg_arr_t, avg_serv_t); break;
            case 2: srtf_sim(avg_arr_t, avg_serv_t); break;
            case 3: hrrn_sim(avg_arr_t, avg_serv_t); break;
            case 4: rr_sim(avg_arr_t, avg_serv_t, q_bit); break;
        }
        generate_report();
    } else{
        cout << "Input all parameter  fields: " << argv[0] << " scheduler type - avg arrival rate - avg service time - quantum interval" << endl;
    }
    return 0;
}
