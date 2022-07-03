#include "MOORA.h"
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <fstream>
#include <stdio.h>  
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <math.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>  
#include <stdlib.h>
#include <limits.h>
using namespace std;
void MOORA::MOORAPRO (int npkts, int sennodes,double power, double bandwidth, double q, double SNR, double distance,int initPos) {
	NUMBEROFpkts 	= npkts;
	NUMBEROFnodes 	= sennodes;
	power 			= power;
	bandwidth 			= bandwidth;
	Q 				= q;
	RO 				= SNR;
	Distance 			= distance;
	INITIALPOS		= initPos;

}
void MOORA::init () {
	CLUSTER 			= new int*[NUMBEROFnodes];
	nodes 			= new double*[NUMBEROFnodes];
	DATA 		= new double*[NUMBEROFnodes];
	DELTADATA = new double*[NUMBEROFnodes];
	PROBS 			= new double*[NUMBEROFnodes-1];
	for(int i=0; i<NUMBEROFnodes; i++) {
		CLUSTER[i] 			= new int[NUMBEROFnodes];
		nodes[i] 			= new double[2];
		DATA[i] 		= new double[NUMBEROFnodes];
		DELTADATA[i] 	= new double[NUMBEROFnodes];
		PROBS[i] 			= new double[2];
		for (int j=0; j<2; j++) {
			nodes[i][j] = -1.0;
			PROBS[i][j]  = -1.0;
		}
		for (int j=0; j<NUMBEROFnodes; j++) {
			CLUSTER[i][j] 			= 0;
			DATA[i][j] 		= 0.0;
			DELTADATA[i][j] 	= 0.0;
		}
	}	
	ROUTES = new int*[NUMBEROFpkts];
	for (int i=0; i<NUMBEROFpkts; i++) {
		ROUTES[i] = new int[NUMBEROFnodes];
		for (int j=0; j<NUMBEROFnodes; j++) {
			ROUTES[i][j] = -1;
		}
	}	
	BESTLENGTH = (double) INT_MAX;
	BESTROUTE  = new int[NUMBEROFnodes];
	for (int i=0; i<NUMBEROFnodes; i++) {
		BESTROUTE[i] = -1;	
	}
}
void MOORA::connectnodes (int mobi, int mobj) {
	CLUSTER[mobi][mobj] = 1;
	DATA[mobi][mobj] = 1 * Distance;
	CLUSTER[mobj][mobi] = 1;
	DATA[mobj][mobi] = DATA[mobi][mobj];
}
void MOORA::setFOGPOSITION (int mob, double x, double y) {
	nodes[mob][0] = x;
	nodes[mob][1] = y;
}
void MOORA::print () {
	for (int i=0; i<NUMBEROFnodes; i++) {
		printf("%5d   ", i);
	}
	cout << endl << "- | ";
	for (int i=0; i<NUMBEROFnodes; i++) {
		cout << "--------";
	}
	cout << endl;
	for (int i=0; i<NUMBEROFnodes; i++) {
		cout << i << " | ";
		for (int j=0; j<NUMBEROFnodes; j++) {
			if (i == j) {
				printf ("%5s   ", "x");
				continue;
			}
			if (exists(i, j)) {
				printf ("%7.3f ", DATA[i][j]);
			}
			else {
				if(DATA[i][j] == 0.0) {
					printf ("%5.0f   ", DATA[i][j]);
				}
				else {
					printf ("%7.3f ", DATA[i][j]);
				}
			}
		}
		cout << endl;
	}
	cout << endl;
}
double MOORA::distance (int mobi, int mobj) {
	return (double) 
		sqrt (pow (nodes[mobi][0] - nodes[mobj][0], 2) + 
 			  pow (nodes[mobi][1] - nodes[mobj][1], 2));
}
bool MOORA::exists (int mobi, int mobc) {
	return (CLUSTER[mobi][mobc] == 1);
}
bool MOORA::vizited (int pktk, int c) {
	for (int l=0; l<NUMBEROFnodes; l++) {
		if (ROUTES[pktk][l] == -1) {
			break;
		}
		if (ROUTES[pktk][l] == c) {
			return true;
		}
	}
	return false;
}
double MOORA::BOF (int mobi, int mobj, int pktk) {
	double ETAij = (double) pow (1 / distance (mobi, mobj), bandwidth);
	double TAUij = (double) pow (DATA[mobi][mobj],   power);

	double sum = 0.0;
	for (int c=0; c<NUMBEROFnodes; c++) {
		if (exists(mobi, c)) {
			if (!vizited(pktk, c)) {
				double ETA = (double) pow (1 / distance (mobi, c), bandwidth);
				double TAU = (double) pow (DATA[mobi][c],   power);
				sum += ETA * TAU;
			}	
		}	
	}
	return (ETAij * TAUij) / sum;
}
double MOORA::length (int pktk) {
	double sum = 0.0;
	for (int j=0; j<NUMBEROFnodes-1; j++) {
		sum += distance (ROUTES[pktk][j], ROUTES[pktk][j+1]);	
	}
	return sum;
}
int MOORA::mob () {
	double xi = 10;
	int i = 0;
	double sum = PROBS[i][0];
	while (sum < xi) {
		i++;
		sum += PROBS[i][0];
	}
	return (int) PROBS[i][1];
}
void MOORA::route (int pktk) {
	ROUTES[pktk][0] = INITIALPOS;
	for (int i=0; i<NUMBEROFnodes-1; i++) {		
		int mobi = ROUTES[pktk][i];
		int count = 0;
		for (int c=0; c<NUMBEROFnodes; c++) {
			if (mobi == c) {
				continue;	
			}
			if (exists (mobi, c)) {
				if (!vizited (pktk, c)) {
					PROBS[count][0] = BOF (mobi, c, pktk);
					PROBS[count][1] = (double) c;
					count++;
				}

			}
		}
		

		if (0 == count) {
			return;
		}
		
		ROUTES[pktk][i+1] = mob();
	}
}
int MOORA::valid (int pktk, int iteration) {
	for(int i=0; i<NUMBEROFnodes-1; i++) {
		int mobi = ROUTES[pktk][i];
		int mobj = ROUTES[pktk][i+1];
		if (mobi < 0 || mobj < 0) {
			return -1;	
		}
		if (!exists(mobi, mobj)) {
			return -2;	
		}
		for (int j=0; j<i-1; j++) {
			if (ROUTES[pktk][i] == ROUTES[pktk][j]) {
				return -3;
			}	
		}
	}
	
	if (!exists (INITIALPOS, ROUTES[pktk][NUMBEROFnodes-1])) {
		return -4;
	}
	
	return 0;
}
void MOORA::updateDATA () {
	for (int k=0; k<200; k++) {
		double rlength = length(k);
		for (int r=0; r<NUMBEROFnodes-1; r++) {
			int mobi = ROUTES[k][r];
			int mobj = ROUTES[k][r+1];
			DELTADATA[mobi][mobj] += Q / rlength;
			DELTADATA[mobj][mobi] += Q / rlength;
		}
	}
	for (int i=0; i<NUMBEROFnodes; i++) {
		for (int j=0; j<NUMBEROFnodes; j++) {
			DATA[i][j] = (1 - RO) * DATA[i][j] + DELTADATA[i][j];
			DELTADATA[i][j] = 0.0;
		}	
	}
}
void MOORA::Clustering (int ROUNDSS) {
	for (int rounds=1; rounds<=ROUNDSS; rounds++) {
		for (int k=0; k<200; k++) {			
			double rlength = length(k);
			if (rlength < BESTLENGTH) {
				BESTLENGTH = rlength;
				for (int i=0; i<NUMBEROFnodes; i++) {
					BESTROUTE[i] = ROUTES[k][i];
				}
			}				
		}		
		updateDATA ();
		for (int i=0; i<200; i++) {
			for (int j=0; j<NUMBEROFnodes; j++) {
				ROUTES[i][j] = -1;
			}		}			}}
