#ifndef MOORA_H
#define MOORA_H
class MOORA {
public:
	void MOORAPRO (int npkts, int sennodes, double power, double bandwidth, double q, double SNR, double distance,int initPos);
	void init ();	
	void connectnodes (int mobi, int mobj);
	void setFOGPOSITION (int mob, double x, double y);
	void print();
	void Clustering (int ITERATIONS);
	double distance (int mobi, int mobj);
	bool exists (int mobi, int mobc);
	bool vizited (int pktk, int c);
	double BOF (int mobi, int mobj, int pktk);
	double length (int pktk);
	int mob ();
	void route (int pktk);
	int valid (int pktk, int iteration);
	void updateDATA ();
	int NUMBEROFpkts, NUMBEROFnodes, INITIALPOS;
	double power, bandwidth, Q, RO, Distance;
	double BESTLENGTH;
	int *BESTROUTE;
	int **CLUSTER, **ROUTES;
	double **nodes, **DATA, **DELTADATA, **PROBS;	
};
#endif 


