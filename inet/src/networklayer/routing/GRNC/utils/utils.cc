#include <iostream>
#include <iostream>
#include <string>
#include <string.h>
#include "utils.h"
using namespace std;
Define_Module(UTILS);
void UTILS::print_matrix(double m[100][100], int rows, int columns){
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            cout << m[i][j] << "\t";
        }
        cout << endl;
    }
}

