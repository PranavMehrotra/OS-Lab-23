#include "defs.h"

void sigusr1_handler(int sig){
    signal(SIGUSR1 , sigusr1_handler);
    alarm(0);
    return;
}

void sigalrm_handler(int sig){
    signal(SIGALRM , sigalrm_handler);
    return;
}

void signal_blocker(int sig,int state){
    sigset_t sig_set;
    sigemptyset(&sig_set);
    sigaddset(&sig_set , sig);
    sigprocmask(state , &sig_set , NULL);
}

ostream& operator<<(ostream& os, const Room& room){
    os << "Room " << room.room_id << " : " << room.guest_id << " " << room.start_time << " " << room.tot_duration << " " << room.occupancy << "\n";
    return os;
}

int gen_rand(int min, int max){
    if(max<min) return 0;
    return min + (rand() % (max - min + 1));
}

bool cmp::operator()(const pair<int,Room> &a, const pair<int,Room> &b) const{
    if(a.first == b.first){
        if(a.second.occupancy == b.second.occupancy){
            return a.second.room_id < b.second.room_id;
        }
        return a.second.occupancy < b.second.occupancy;
    }
    return a.first < b.first;
}