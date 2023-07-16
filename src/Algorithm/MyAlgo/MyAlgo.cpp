#include "MyAlgo.h"

MyAlgo::MyAlgo(string filename, int request_time_limit, int node_time_limit, double swap_prob, double entangle_alpha)
    :AlgorithmBase(filename, "MyAlgo", request_time_limit, node_time_limit, swap_prob, entangle_alpha){
    if(DEBUG) cerr<<"new MyAlgo"<<endl;
}

MyAlgo::~MyAlgo(){
    if(DEBUG) cerr << "delete MyAlgo" << endl;
}

void MyAlgo::entangle(){
    AlgorithmBase::base_entangle();
}

void MyAlgo::swap(){
     AlgorithmBase::base_swap();
}

void MyAlgo::send(){
     AlgorithmBase::base_send();
}

void MyAlgo::next_time_slot(){
     AlgorithmBase::base_next_time_slot();
}

void MyAlgo::initialize(){
    M = graph.get_size() + graph.get_num_of_edge() + requests.size();              //M=V+E+I
    delta = (1 + epsilon) * pow(((1 + epsilon) * M), (-1 / epsilon));         

    for(int i = 0; i < graph.get_size(); i++){
        alpha.emplace_back(delta / graph.Node_id2ptr(i)->get_memory_cnt());       //alpha=delta/Mu

        vector<int> temp = graph.get_neighbors_id(i);                             //beta=delta/Cuv
        for(auto it: temp){
            if(i < it){
                beta[make_pair(i,it)] = delta / (graph.get_channel_size(i, it));
            }
            else{
                beta[make_pair(it,i)] = delta / (graph.get_channel_size(i, it));
            }
        }
    }

    for(unsigned  i = 0; i < requests.size(); i++){                               //tau=delta/ri
        tau.emplace_back(delta / requests[i].get_send_limit());
    }
    Y.resize(requests.size() + 5);
    for(int i = 0; i < graph.get_size(); i++){
        vector<int> temp = graph.get_neighbors_id(i);                             
        for(auto it: temp){
            if(i < it){                                                           //X=alpha(left_node)+alpha(right_node)+beta(edge between them)
                X[{i, it}] = alpha[i] + alpha[it] + beta[{i, it}];
                X[{it, i}] = alpha[i] + alpha[it] + beta[{i, it}];
            }
            else{
                X[{it, i}] = alpha[it] + alpha[i] + beta[{it, i}];
                X[{i, it}] = alpha[it] + alpha[i] + beta[{it, i}];
            }
            for(unsigned  j = 0;j < requests.size(); j++){                       //Y=-ln(edge)-ln(repeater_1^(1/2))-ln(repeater_2^(1/2))
                int src = requests[j].get_source();
                int des = requests[j].get_destination();
                double ent_p = exp(graph.Node_id2ptr(i)->distance(*graph.Node_id2ptr(it))*(-graph.get_entangle_alpha()));
                if(i<it){
                    if(i != src && i != des && it != src && it != des){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1)))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1)))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                    }
                    else if((i == src && it != des) || (i == des && it != src)){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                    }
                    else if((i == src && it != des) || (i == des && it != src)){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1))));
                    }
                    else{
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1)));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1)));
                    }
                }
                else{
                    if(i != src && i != des && it != src && it != des){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1)))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1)))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                    }
                    else if((i == src && it != des) || (i == des && it != src)){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(it)->get_swap_prob())/log(exp(1))));
                    }
                    else if((i == src && it != des) || (i == des && it != src)){
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1))));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1))) - (log(sqrt(graph.Node_id2ptr(i)->get_swap_prob())/log(exp(1))));
                    }
                    else{
                        Y[j][{i, it}] = -(log(ent_p)/log(exp(1)));
                        Y[j][{it, i}] = -(log(ent_p)/log(exp(1)));
                    }                  
                }

            }
        }
    }
}

vector<int> MyAlgo::Dijkstra(int src, int dst){ 
    const double INF = numeric_limits<double>::infinity();
    int n = graph.get_size();
    vector<double> distance(n, INF);
    vector<int> parent(n, -1);
    vector<bool> used(n, false);
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

    distance[dst] = 0;
    pq.push({0, dst});
    while(!pq.empty()) {
        int cur_node = pq.top().second;
        pq.pop();
        if(used[cur_node]) continue;
        used[cur_node] = true;
        for(int neighbor : graph.get_neighbors_id(cur_node)) {
            if(distance[cur_node] + X[{cur_node, neighbor}] < distance[neighbor]) {
                distance[neighbor] = distance[cur_node] + X[{cur_node, neighbor}];
                parent[neighbor] = cur_node;
                pq.push({distance[neighbor], neighbor});
            }
        }
    }

    if(distance[src] >= INF) return{};
/*
    int cur_node = src;
    vector<int> path;
    while(cur_node != -1) {
        path.push_back(cur_node);
        cur_node = parent[cur_node];
    }

    reverse(path.begin(), path.end());
*/
    return parent;
}       
    //     3---4
    //  /  |  /
    // /   | / 
    // 2---1/
    //  \  |
    //   \ |
    //     0

vector<int> MyAlgo::separation_oracle(int req_no, double &req_Us){     
    vector<int> SPT;                  //nodes' parent in the spanning tree
    vector<int> best_path;
    double best_len; 
    int src = requests[req_no].get_source();
    int dst =  requests[req_no].get_destination();

    double brute_min=numeric_limits<double>::infinity();
    vector<int>brute_path;
    for(auto it:all_source_target_path[req_no]){            //brute sort U_count
        double c=0;
        double r=0;
        for(unsigned int i=0;i<it.size()-1;i++){
            c += X[{it[i],it[i+1]}];               
            r += Y[req_no][{it[i],it[i+1]}]; 
        }
        if(c * exp(r) < brute_min){
            brute_min = c * exp(r);
            brute_path = it;
        }
    }
    cout<<"\n[BRUTE]req:"<<req_no<<" ";
    for(auto it:brute_path){
        cout<<it<<"->";
    }
    cout<<":"<<brute_min<<endl;

    SPT = Dijkstra(src, dst);                               //the first SPT is get by dijkstra
    int cur_node = src;                                     //counting the first path's U(X,Y)=c* e^r
    double c = 0;                                           //c=SUM[u,v]:alpha(u)+alpha(v)+beta(u,v)==X[u,v]
    double r = 0;                                           //r=SUM[u,v]:-ln(Pr(u,v))==Y[u,v]
    while(cur_node != dst){
        if(cur_node < SPT[cur_node]){                       //[can alter]no need if else
            c += X[{cur_node,SPT[cur_node]}];               
            r += Y[req_no][{cur_node,SPT[cur_node]}];
        }
        else{
            c += X[{SPT[cur_node],cur_node}];  
            r += Y[req_no][{SPT[cur_node],cur_node}];           
        }
        best_path.push_back(cur_node);
        cur_node = SPT[cur_node];
    } 
    best_path.push_back(dst);
    best_len = c * exp(r);
    req_Us = best_len;
    cout << "origin path: ";
    for(auto p : best_path){
            cout << p << "->";
    }
    cout << endl;
    map<pair<int, int>, bool> used_edge;
    vector<int> new_path;   
    pair<int,int> new_edge;
    while(1){
        double minimum = numeric_limits<double>::infinity();
        for(int i = 0; i < graph.get_size(); i++){                 //creating many new SPT
            vector<int> neighbors = graph.get_neighbors_id(i);  
            for(auto neighbor : neighbors){
                double temp1 = 0, temp2 = 0;
                if(SPT[i] == neighbor || SPT[neighbor] == i){      // checking used edge or unused
                    continue;   
                }else{                                             // if unused
                    temp1 = X[{i, neighbor}];
                    temp2 = Y[req_no][{i, neighbor}];
                    cout <<"(i, j): "<< i << " "<< neighbor <<" C(i, j): " << temp1 << " r(i, j)" << temp2 << endl;
                    int cur_node = i;
                    cout << "Cp_j" << endl;
                    while(cur_node != dst){
                        cout  << "edge: " << cur_node << " "<< SPT[cur_node] << endl;
                        cout  << "C: " << X[{cur_node, SPT[cur_node]}] << " R: " << Y[req_no][{cur_node, SPT[cur_node]}] << endl;
                        temp1 += X[{cur_node, SPT[cur_node]}];
                        temp2 += Y[req_no][{cur_node, SPT[cur_node]}];
                        cur_node = SPT[cur_node];
                    } 
                    cout << "Cp_i" << endl;
                    cur_node = neighbor;
                    while(cur_node != dst){
                        cout  << "edge: " << cur_node <<" "<<SPT[cur_node] << endl;
                        cout  << "C: " << X[{cur_node, SPT[cur_node]}] << " R: " << Y[req_no][{cur_node, SPT[cur_node]}] << endl;
                        temp1 -= X[{cur_node, SPT[cur_node]}];
                        temp2 -= Y[req_no][{cur_node, SPT[cur_node]}];
                        cur_node = SPT[cur_node];
                    }       
                    if(temp2 < 0 && temp1 > 0){               // we need the smallest edge to change the SPT
                        if(minimum > - temp1 / temp2){
                            cout  << "change edge: " <<  i <<" "<< neighbor << endl;
                            new_edge = {i, neighbor};
                            minimum = - temp1 / temp2;
                            cout << "Cost: " << minimum << endl;
                        }
                    }
                }
            }
        }        // 找到最小的 edge 

        cout << "minimum: " << minimum << endl;
        cout << "fabs: " << fabs(minimum - numeric_limits<double>::infinity()) << endl;
        if(minimum == numeric_limits<double>::infinity()){   //原本設計是有break,但之後用不到
            break;
        }else{
            new_path.clear();
        }
        
        cur_node = new_edge.first;
        int cur_node2 = new_edge.second;
        used_edge[new_edge] = true;
        while(cur_node != dst && cur_node2 != dst){
            cur_node=SPT[cur_node];
            cur_node2=SPT[cur_node2];
        }
        if(cur_node == dst){
            SPT[new_edge.second] = new_edge.first;
        }
        else{
            SPT[new_edge.first] = new_edge.second;
        }

        cur_node = src;                                   
        while(cur_node != dst) {
            new_path.push_back(cur_node);
            cur_node = SPT[cur_node];
            
        }       
        new_path.push_back(dst);

        double new_len = 0;                                         //counting the new path's U(X,Y)=c* e^r
        c = 0;
        r = 0;
        for(unsigned int i = 0; i < new_path.size() - 1; i++){
            if(new_path[i] < new_path[i+1]){                        //[can alter]no need if else
                c += X[{new_path[i], new_path[i+1]}];
                r += Y[req_no][{new_path[i], new_path[i+1]}];
            }
            else{
                c += X[{new_path[i+1], new_path[i]}];  
                r += Y[req_no][{new_path[i+1], new_path[i]}];           
            }
            //cout<<"PATH:["<<new_path[i]<<" "<<new_path[i+1]<<"] with tot "<< c <<" / " << r <<endl;
        }
        new_len =  c * exp(r);
        if(new_len < best_len){
            best_len = new_len;
            req_Us = best_len;
            
            best_path = new_path;                                            //路線修改,新的spt產生
        } 
        
       
    }
    if(best_path != brute_path){                                           //checking brute && best
        cout<<"DIFF!!!\n";
    }

    cout << "Best path: ";
    for(auto p : best_path){
        cout <<p << " ";
    }
    cout << endl;
    cout << "U: " << best_len << endl;
        
    return best_path;  
                                                    
}

void MyAlgo::find_bottleneck(vector<int> path, int req_no){
    
    double min_s_u = numeric_limits<double>::infinity();
    double min_s_uv = numeric_limits<double>::infinity();
    double s_i = requests[req_no].get_send_limit();                             //request[no] min
    vector<double> s_u(graph.get_size() + 5);
    vector<double> s_uv(graph.get_size() + 5);                                               

   
    for(unsigned int i = 0; i < path.size(); i++){
        if(i == 0 || path.size() - 1)
            s_u[path[i]] = graph.Node_id2ptr(path[i])->get_memory_cnt();        //if src or dst,then only cost 1
        else
            s_u[path[i]] = graph.Node_id2ptr(path[i])->get_memory_cnt() / 2;    //else cost 2
        if(s_u[path[i]] < min_s_u)
            min_s_u = s_u[path[i]];
    }

    for(unsigned int i = 0; i < path.size() - 1; i++){
        s_uv[i] = graph.get_channel_size(path[i], path[i+1]);                   //channel min
        if(s_u[i] < min_s_uv)
            min_s_uv = s_uv[i];
    }

    double s = min(min_s_u, min(min_s_uv, s_i));
   
    if(x_i_p.find(path) != x_i_p.end())                                         //add flow to path
        x_i_p[path] += s;
    else
        x_i_p[path] = s;
    
    for(auto id : path){                                                        //alter alpha,beta,tau
        alpha[id] = alpha[id] * (1 + epsilon * s / s_u[id]);
    }

    for(unsigned int i = 0; i < path.size() - 1; i++){
        beta[{path[i], path[i+1]}] = beta[{path[i], path[i+1]}] * (1 + epsilon * s / s_uv[i]);
    }

    tau[req_no] = tau[req_no] * (1 + epsilon * s / s_i);    

    //now changing the X
    for(unsigned int i = 0; i < path.size() -1; i++){
        if(path[i]<path[i+1]){
            X[{path[i],path[i+1]}] = alpha[path[i]] + alpha[path[i+1]] + beta[{path[i], path[i+1]}];
            X[{path[i+1],path[i]}] = alpha[path[i]] + alpha[path[i+1]] + beta[{path[i], path[i+1]}];
        }
        else{
            X[{path[i],path[i+1]}] = alpha[path[i]] + alpha[path[i+1]] + beta[{path[i+1], path[i]}];
            X[{path[i+1],path[i]}] = alpha[path[i]] + alpha[path[i+1]] + beta[{path[i+1], path[i]}];   
        }

    }
}

double MyAlgo::changing_obj(){
    double temp_obj = 0.0;
    for(unsigned int i = 0; i < alpha.size(); i++){
        temp_obj += alpha[i] * graph.Node_id2ptr(i)->get_memory_cnt();
    }
    
    for(auto it : beta){
        temp_obj += it.second * graph.get_channel_size(it.first.first, it.first.second);
    }

    for(unsigned int i = 0;i < requests.size(); i++){
        temp_obj += tau[i] * requests[i].get_send_limit();
    }
    return temp_obj;
}

void MyAlgo::find_violate(){
    vector<int> used_memory(graph.get_size());
    map<vector<int>, double> used_channel;
    map<pair<int, int>, int> used_request;

    for(auto &it : x_i_p){
        vector<int> path = it.first;
        int src = path[0];
        int dst = path.back();
    
        if(used_request.find({src, dst}) != used_request.end())
            used_request[{src, dst}] += it.second;
        else
            used_request[{src, dst}] = it.second;
        
        for(unsigned int i = 0; i < path.size() - 1; i++){
            used_memory[path[i]] += it.second;                         //memory add
            used_memory[path[i+1]] += it.second;
            if(path[i] < path[i+1]){
                auto iter = used_channel.find({path[i], path[i+1]});
                if(iter != used_channel.end()){    //channel add
                    used_channel[{path[i], path[i+1]}] += it.second;
                }
                else{
                    used_channel[{path[i], path[i+1]}] = it.second;
                }
            }
            else{
                auto iter = used_channel.find({path[i+1], path[i]});
                if(iter != used_channel.end()){
                    used_channel[{path[i+1], path[i]}] += it.second;
                }
                else{
                    used_channel[{path[i+1], path[i]}] = it.second;
                }  
            }
        }
    }


    double max_magni = 0.0;
    double cur_magni;
    for(int i = 0; i < graph.get_size(); i++){
        cur_magni = used_memory[i] / graph.Node_id2ptr(i)->get_memory_cnt();
        if(cur_magni > max_magni){
            max_magni = cur_magni;
        }
    }
    for(auto it : used_channel){
        cur_magni = it.second / graph.get_channel_size(it.first[0],it.first[1]);
        if(cur_magni > max_magni){
            max_magni = cur_magni;
        }
    }

    for(auto it : used_request){
        cur_magni = it.second / requests[0].get_send_limit();
        if(cur_magni > max_magni){
            max_magni = cur_magni;
        }
    }
    //cout << "Magnification:" << max_magni << endl;

    for(auto &x : x_i_p){
        cout << "before: " << x.second;
        x.second /= max_magni;
        cout << " after: " << x.second << endl;
    }
    //check memory_and_channel
    /*
    for(unsigned int i=0;i<used_memory.size();i++){
        cout<<i<<" with memory "<<used_memory[i]<<endl;
    }
    for(auto it:used_channel){
        cout<<"["<<it.first[0]<<","<<it.first[1]<<"]:"<<it.second<<endl;
    }
    */
}

vector<map<vector<int>, int>> MyAlgo::rounding(){
    vector<map<vector<int>, double>> each_request(requests.size());
    vector<map<vector<int>, int>> I_request(requests.size());
    for(auto it : x_i_p){
        vector<int> path = it.first;
        int src = path[0];
        int dst = path.back();
        for(unsigned int i = 0; i < requests.size(); i++){
            if(src == requests[i].get_source() && dst == requests[i].get_destination()){
                each_request[i][path] = it.second;
                break;
            }
        }
    }

    for(unsigned int i = 0; i < each_request.size(); i++){
        for(auto it:each_request[i]){
            vector<int>undistri_path =it.first;
            cout<<"[undistri] ";
            for(auto it2:undistri_path){
                cout<<it2<<" ";
            }
            cout<<"     Qubits:"<<it.second<<endl;
        }
    }

    for(unsigned int i = 0; i < requests.size(); i++){
        double total_prob = 0;
        double used_prob = 0;
        int used_I=0;
        int distri_I=0;
        vector<double>accumulate;
        accumulate.push_back(0.0);                                              // [0,a1,a2,...,0]
        for(auto it : each_request[i]){                    
            double frac_prob;

            int i_prob = it.second;                                             //每個path先取整數部分=>確定分配
            I_request[i][it.first] = i_prob;
            used_I+=i_prob;

            frac_prob = it.second - i_prob;                                     //total_prob代表random區間,丟進accumulate
            total_prob += frac_prob;
            accumulate.push_back(total_prob);
            used_prob += it.second;
        }
        used_I += (int)(requests[i].get_send_limit()- used_prob);               //unused_I=取底[ri - sum(request.I) - (unused.I)]
        distri_I=requests[i].get_send_limit()-used_I;

        accumulate.push_back(0.0);
        cout<<"total_prob:"<<total_prob<<" distri_I:"<<distri_I<<endl;
        cout<<"accumulate:";
        for(auto it:accumulate){
            cout<<it<<" ";
        }
        cout<<endl;
        for(int j = 0; j < distri_I; j++){
            random_device rd;  
            mt19937 gen(rd()); 
            uniform_real_distribution<double> dis(0.0, total_prob);
            double temp = dis(gen);
            for(unsigned int k = 0; k < accumulate.size() - 1; k++){
                cout<<"ramdom:"<<temp<<endl;
                if(temp > accumulate[k] && temp < accumulate[k+1]){
                    unsigned int index = 0;
                    for(auto it : each_request[i]){
                        if(index == k){
                            I_request[i][it.first]++;
                            //cout<<"random prob="<<temp<<" so "<<k<<" added!\n";
                            break;
                        }
                        index += 1;
                    }
                    break;
                }
            }
        }
    }
    for(unsigned int i = 0; i < I_request.size(); i++){
        for(auto it:I_request[i]){
            vector<int>Final_path =it.first;
            for(auto it2:Final_path){
                cout<<it2<<" ";
            }
            cout<<"     Qubits:"<<it.second<<endl;
        }
    }
    return I_request;
}

void MyAlgo::dfs(int src, int dst, vector<vector<int>> &ans, vector<int> &path, vector<bool> &visited){
        //base case
    visited[src] = true;
    path.push_back(src);
    if(src == dst){
        ans.push_back(path);
        // cout << "allpath: ";
        // for(auto p : path){
        //     cout << p << "->";
        // }
        // cout << endl;

    } 
    else{
        for(auto i : graph.get_neighbors_id(src)){ 
            if(!visited[i]){
                dfs(i, dst, ans, path, visited);
            }
        }
    }
    visited[src] = false;
    path.pop_back();

}
vector<vector<int>> MyAlgo::allPathsSourceTarget(int src, int dst){
    vector<vector<int>> ans;
    vector<int> path;
    vector<bool> visited(graph.get_size());
    for(int i = 0; i < graph.get_size(); i++){
        visited[i] = false;
    }
    dfs(src, dst, ans, path, visited);
    return ans;
}

void MyAlgo::path_assignment(){
    initialize();
    
    for(unsigned int i = 0; i < requests.size(); i++){
        int src = requests[i].get_source();
        int dst = requests[i].get_destination();
        all_source_target_path.push_back(allPathsSourceTarget(src, dst));
    }

    
    double obj = M * delta;
    vector<int> best_path;
    vector<int> cur_path;
    double U;
    while(obj < 1){ // 是否在裡面做完 entangele, swap, send, separation_oracle 一個 SD 只找一條path?
        int req_no = 0;
        double smallest_U = numeric_limits<double>::infinity();
        //cout<<"\n------New round-------\n";
        for(unsigned int i = 0; i < requests.size(); i++){
            cur_path =  separation_oracle(i, U);
            //cout << "smallest_U: " << smallest_U << " U: " << U << "\n\n"; 
            if(U < smallest_U){
                smallest_U  = U;
                best_path = cur_path;
                req_no = i;
            }
        }
        find_bottleneck(best_path, req_no);
        //cout<<"End find_bottle\n";
        obj = changing_obj();
        cout<<"changing_obj obj: " << obj << endl ;
    }
    find_violate();
    vector<map<vector<int>, int>>path = rounding();

    for(unsigned int i = 0; i < path.size(); i++){
        for(auto p : path[i]){
            assign_resource(p.first, p.second, i);
        }
    }
}   

