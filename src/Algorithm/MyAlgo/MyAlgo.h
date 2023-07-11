#ifndef __MYALGO_H
#define __MYALGO_H

#include <iostream>
#include <algorithm>
#include <queue>
#include <limits>
#include <string>
#include <cmath>
#include "../AlgorithmBase/AlgorithmBase.h"
#include "../../Request/WholeRequest.h"
#include "../../Network/Graph/Graph.h"
#include "../../config.h"

using namespace std;

class MyAlgo:public AlgorithmBase {

private:
    map<pair<int,int>, double> X;
    vector<map<pair<int,int>, double>> Y;
    vector<double> alpha;
    map<vector<int>, double> x_i_p;
    map<pair<int,int>, double> beta;
    double epsilon;
    double delta;
    double M;
    vector<double> tau;                
public: 
    map<int, int> num_of_path_count;
    map<int, int> path_length_encode;
    map<int, int> path_length_cnt;
    vector<int> separation_oracle(int req_no, double &U);
    vector<int> Dijkstra(int src, int dst);
    void path_assignment();
    void entangle();
    void swap();
    void send();
    void next_time_slot();
    void find_bottleneck(vector<int>, int req_no);
    void initialize();
    double changing_obj();
    MyAlgo(string filename, int request_time_limit, int node_time_limit, double swap_prob, double entangle_alpha);
    ~MyAlgo();
};

#endif