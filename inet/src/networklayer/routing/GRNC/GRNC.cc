#include <iostream>
#include <string>
#include <string.h>
#include "utils/utils.h"
#include "GRNC.h"
using namespace std;
Define_Module(GRNC);
#define MATRIX_ROW 6
#define MATRIX_COLUMN 6
#define STATE_NUM 6
#define ACTION_NUM 6
#define DES_STATE 5
#define MAX_EPISODE 1000
#define alpha 0.8
double R[100][100] = {{-1, -1, -1, -1, 0, -1},
                     {-1, -1, -1, 0, -1, 100},
                     {-1, -1, -1, 0, -1, -1},
                     {-1, 0, 0, -1, 0, -1},
                     {0, -1, -1, 0, -1, 100},
                     {-1, 0, -1, -1, 0, 100}};
double Q[100][100];
int possible_action[10];
int possible_action_num;
void GRNC::get_possible_action(double R[100][100], int state, int possible_action[10]){
    possible_action_num = 0;
    for(int i = 0; i < ACTION_NUM; i++){
        if (R[state][i] >= 0){
            possible_action[possible_action_num] = i;
            possible_action_num++;
        }
    }
}
double GRNC::get_max_q(double Q[100][100], int state){
    double temp_max = 0;
    for (int i = 0; i < ACTION_NUM; ++i) {
        if ((R[state][i] >= 0) && (Q[state][i] > temp_max)){
            temp_max = Q[state][i];
        }
    }
    return temp_max;
}
int GRNC::episode_iterator(int init_state, double Q[100][100], double R[100][100]){
    double Q_before, Q_after;
    int next_action;
    double max_q;
    int step=0;
    while (1){
        cout << "-- step " << step << ": initial state: " << init_state << endl;
        memset(possible_action, 0, 10*sizeof(int));
        get_possible_action(R, init_state, possible_action);
        next_action = possible_action[rand()%possible_action_num];
        cout << "-- step " << step << ": next action: " << next_action << endl;
        max_q = get_max_q(Q, next_action);
        Q_before = Q[init_state][next_action];
        Q[init_state][next_action] = R[init_state][next_action] + alpha * max_q;
        Q_after = Q[init_state][next_action];
        if (next_action == DES_STATE){
            init_state = rand()%STATE_NUM;
            break;
        }else{
            init_state = next_action;
        }
        step++;
    }
    return init_state;
}

int GRNC::inference_best_action(int now_state, double Q[100][100]){
    double temp_max_q=0;
    int best_action=0;
    for (int i = 0; i < ACTION_NUM; ++i) {
        if (Q[now_state][i] > temp_max_q){
            temp_max_q = Q[now_state][i];
            best_action = i;
        }
    }
    return best_action;
}
void GRNC::run_Classification(int init_state){
    int initial_state = init_state;
    srand((unsigned)time(NULL));
    cout << "[INFO] start classification..." << endl;
    for (int i = 0; i < MAX_EPISODE; ++i) {
        cout << "[INFO] Episode: " << i << endl;
        initial_state = episode_iterator(initial_state, Q, R);
        cout << "-- updated : " << endl;
    }
}

