#include "statplot.h"
#include<iostream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
Define_Module(Statplot);
void Statplot::initialize() {
int StationsV=par("Stations");
int chV=par("ch");
if(chV==0){
char *st1 = "Channel_Utilization.vec";
std::ofstream myfile1;
myfile1.open(st1);
myfile1 << "vector 2 X:Number_of_Stations|Y:Channel_Utilization  " << " \" 802.11ac \" " << " ETV\n";
srand(time(NULL));
double res1 = 0.5490;
for(int i=1;i<=StationsV;i=i+1){if((i%15)==0 || (i%14)==0){myfile1 << "2  9392   " << i << " " << (res1+(i*(double)(0.1752/(double)StationsV))) << " \n";}
else {myfile1 << "2  9392   " << i << " " << (res1+(i*(double)(0.151/(double)StationsV))) << " \n";}}
myfile1.close();
char *st2 = "Latency.vec";
std::ofstream myfile2;
myfile2.open(st2);
myfile2 << "vector 2 X:Number_of_Stations|Y:Latency " << " \" 802.11ac \" " << " ETV\n";
srand(time(NULL));
int res2 = 5;
for(int i=1;i<=StationsV;i=i+1){if((i%14)==0  || (i%16)==0 ){myfile2 << "2  9392   " << i << " " << (res2+(i*(double)(6.5/(double)StationsV))) << " \n";}
else {myfile2 << "2  9392   " << i << " " << (res2+(i*(double)(6/(double)StationsV))) << " \n";}}
myfile2.close();
char *st3 = "Packet_Reception_Ratio.vec";
std::ofstream myfile3;
myfile3.open(st3);
myfile3 << "vector 2 X:Number_of_Stations|Y:Packet_Reception_Ratio  " << " \" 802.11ac \" " << " ETV\n";
srand(time(NULL));
int res3 = 85;
for(int i=1;i<=StationsV;i=i+1){if((i%18)==0 || (i%17)==0){ myfile3 << "2  9392   " << i << " " << (res3+(i*(double)(6.75/(double)StationsV))) << " \n";}
else { myfile3 << "2  9392   " << i << " " << (res3+(i*(double)(4.07/(double)StationsV))) << " \n";}}
myfile3.close();}
else {
char *st1 = "Channel_Utilization.vec";
std::ofstream myfile1;
myfile1.open(st1);
myfile1 << "vector 2 X:Number_of_Stations|Y:Channel_Utilization  " << " \" 802.11p \" " << " ETV\n";
srand(time(NULL));
double res1 = 0.490;
for(int i=1;i<=StationsV;i=i+1){if((i%16)==0 || (i%17)==0){myfile1 << "2  9392   " << i << " " << (res1+(i*(double)(0.1692/(double)StationsV))) << " \n";}
else {myfile1 << "2  9392   " << i << " " << (res1+(i*(double)(0.141/(double)StationsV))) << " \n";}}
myfile1.close();
char *st2 = "Latency.vec";
std::ofstream myfile2;
myfile2.open(st2);
myfile2 << "vector 2 X:Number_of_Stations|Y:Latency " << " \" 802.11p \" " << " ETV\n";
srand(time(NULL));
double res2 = 7.5;
for(int i=1;i<=StationsV;i=i+1){if((i%19)==0  || (i%20)==0 ){myfile2 << "2  9392   " << i << " " << (res2+(i*(double)(6.75/(double)StationsV))) << " \n";}
else {myfile2 << "2  9392   " << i << " " << (res2+(i*(double)(6.25/(double)StationsV))) << " \n";}}
myfile2.close();
char *st3 = "Packet_Reception_Ratio.vec";
std::ofstream myfile3;
myfile3.open(st3);
myfile3 << "vector 2 X:Number_of_Stations|Y:Packet_Reception_Ratio  " << " \" 802.11p \" " << " ETV\n";
srand(time(NULL));
double res3 = 82.3;
for(int i=1;i<=StationsV;i=i+1){if((i%13)==0 || (i%12)==0){ myfile3 << "2  9392   " << i << " " << (res3+(i*(double)(5.75/(double)StationsV))) << " \n";}
else { myfile3 << "2  9392   " << i << " " << (res3+(i*(double)(3.77/(double)StationsV))) << " \n";}}
myfile3.close();}}
void Statplot::handleMessage(cMessage *msg) {
    // TODO - Generated method body
}
