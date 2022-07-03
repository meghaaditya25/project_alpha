

#ifndef GRNC_H
#define GRNC_H
#include <omnetpp.h>
class GRNC : public cSimpleModule
{
public:
void get_possible_action(double R[100][100], int state, int possible_action[10]);
double get_max_q(double Q[100][100], int state);
int episode_iterator(int init_state, double Q[100][100], double R[100][100]);
int inference_best_action(int now_state, double Q[100][100]);
void run_Classification(int init_state);
};
#endif //GRNC_UTILS_H
