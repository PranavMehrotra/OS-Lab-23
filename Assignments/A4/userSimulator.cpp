#include <bits/stdc++.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "defs.h"
using namespace std;

extern vector<vector<int>> graph;
extern map<int, Node> users;
extern my_semaphore write_logfile;
extern ofstream logfile;

my_semaphore write_shared(1),read_shared(0);
int curr_iter = 0;
vector<vector<Action>> shared(RANDOM_NODE_COUNT);

bool cmp(const vector<Action> &a , const vector<Action> &b){
    return a.size() > b.size();
}

void *userSimulator(void *arg){

    while(1){
        vector<vector<Action>> temp_shared(RANDOM_NODE_COUNT);

        for(int i=0; i<RANDOM_NODE_COUNT; i++){
            int random_node = rand()%users.size();
            // cout << random_node << " " << users[random_node].degree << " " << users[random_node].log_degree << endl;
            int num_actions = ceil(users[random_node].log_degree);
            
            for(int j=0; j<num_actions; j++){
                int action_type = rand()%3;
                long timestamp = time(0);
                Action action(random_node , ++users[random_node].num_action[action_type] , timestamp , action_type);
                users[random_node].wall.push_back(action);     // Push to Wall queue of user
                temp_shared[i].push_back(action);
            }
        }

        sort(temp_shared.begin() , temp_shared.end() , cmp);

        write_shared._wait();
        // write to shared
        curr_iter++;
        for(int i=0;i<RANDOM_NODE_COUNT;i++){
            shared[(i+curr_iter)%RANDOM_NODE_COUNT] = temp_shared[i];         
        }
        read_shared._signal();

        write_logfile._wait();
        // write to log file
        logfile << "---------------------------------------------------------------------------\n";
        logfile << "userSimulator iteration #" << curr_iter << " : " << endl;
        for(int i=0;i<RANDOM_NODE_COUNT;i++){
            for(int j=0;j<temp_shared[i].size();j++) logfile << temp_shared[i][j] << endl;
        }
        write_logfile._signal();

        sleep(SLEEP_SECONDS);
    }

    pthread_exit(NULL);
}