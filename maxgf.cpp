#include<cstdlib>
#include<iostream>
//#include<vector>
//#include<fstream>
#include<string>
#include<sstream>
#include<set>
#include<list>
//#include<ctime>
#include<algorithm>
#include <future>
//#include<map>
//#include<utility>
//#include<queue>
//#include<cmath>
//#include<pthread.h>
//#include<unistd.h>
#include "Graph.h"

using namespace std;

Solution* mission(User* v, Graph* g, int n, int m, int h, int thread_no, int counter, double max);
Solution* thread_mission(list<User*> todo, Graph* g, int n, int m, int h, int thread_no);
void thread_preliminary(list<User*> todo);
set<User*> post_processing(Graph* g, set<User*> &U, double least_obj, int n, int h);
void write_out(string filename, set<User*> U, double opt, double time);

int main(int argc, char* argv[]){
	// network, n, m, h
	string network = "test";
	int n = 1;
	int m = 1;
	int h = 1;
	int n_thread = 1;
	
	if(argc >= 5){
		network = argv[1];
		n = std::stoi(argv[2], NULL);
		m = std::stoi(argv[3], NULL);
		h = std::stoi(argv[4], NULL);
		if(argc >= 6){
			n_thread = std::stoi(argv[5], NULL);
		}
	}
	
	Graph* g = read_social(network);
	cout << "Finish reading" << endl;
	
	clock_t start = clock();
	
	int counter = 0;
	vector<list<User*>> todos;
	for(int i=0; i<n_thread; i++){
		list<User*> todo;
		todos.push_back(todo);
	}
	
	if(n_thread == 1){
		for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
			(*it)->get_sum_of_all_potential_weights();
		}	
	}
	else{
		for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
			int no = counter++ % n_thread;
			todos.at(no).push_back(*it);
		}
		list<future<void>> f;
		for(int i=0; i<todos.size(); i++){
			f.push_back(std::async(std::launch::async, thread_preliminary, todos.at(i)));
		}
		for(list<future<void>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			(*f_it).get();
		}
		
		// init future-related variables
		for(int i=0; i<n_thread; i++){			
			todos.at(i).clear();
		}
		counter = 0;		
	}
	g->sort_users();
	cout << "Preliminary done" << endl;	
	
	double opt = 0.0;
	set<User*> opt_U;
		
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		User* v = *it;
		if(n_thread == 1){
			Solution* s = mission(v, g, n, m, h, 1, counter, opt);			
			if(s != NULL && opt < s->opt){
				opt = s->opt;
				opt_U = s->U;
			}
		}
		else{			
			int no = counter % n_thread;
			todos.at(no).push_back(v);
		}
	    counter++;
	}
	
	if(n_thread > 1){
		list<future<Solution*>> f;
		for(int i=0; i<todos.size(); i++){
			f.push_back(std::async(std::launch::async, thread_mission, todos.at(i), g, n, m, h, i));
		}
		for(list<future<Solution*>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			Solution* s = (*f_it).get();
			if(s != NULL && opt < s->opt){
				opt = s->opt;
				opt_U = s->U;
			}					
		}
	}
	
	
	cout << "Start post processing" << endl;
	opt_U = post_processing(g, opt_U, opt, n, h);

	clock_t end = clock();	
	
	// output
	stringstream ss;
	ss << network << "_" << n << "_" << m << "_" << h << "_MaxGF.out";
	double t_all = double(end-start)/double(CLOCKS_PER_SEC);
	write_out(ss.str(), opt_U, opt, t_all);
}


void thread_preliminary(list<User*> todo){
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		(*it)->get_sum_of_all_potential_weights();
	}
}

Solution* thread_mission(list<User*> todo, Graph* g, int n, int m, int h, int thread_no){
	double obj = 0.0;
	set<User*> U;
	set<Item*> O;
	
	int counter = 0;
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		Solution* s = mission(*it, g, n, m, h, thread_no, counter++, obj);
		if(s != NULL && obj < s->opt){
			obj = s->opt;
			U = s->U;
		}
	}
	
	Solution* ret = new Solution(obj, U, O);
	return ret; 
}

Solution* mission(User* v, Graph* g, int n, int m, int h, int thread_no, int counter, double max){	
	set<User*> H_v = v->get_h_hop_neighborhood(h);
	counter++;
	if(H_v.size() < n){
		return NULL;
	}
	double max_social = 0.0;
	for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
		double v_social = (*it)->get_sum_of_potential_weights(H_v)/2;
		if(v_social >= max_social){
			max_social = v_social;
		}
	}
	if(max_social < max){
		return NULL;
	}

	// MaxGF		
	set<User*> U_MaxGF = MaxGF(H_v, n);
	set<Item*> O;
	cout << "[" << thread_no << "] " << counter << ": " << v->id << " U finished" << endl;
	Solution* ret = new Solution(compute_social_obj(U_MaxGF), U_MaxGF, O);
	
	return ret;					 
}

set<User*> post_processing(Graph* g, set<User*> &U, double least_obj, int n, int h){
	set<User*> ret = U;
	set<User*> all;
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		all.insert(*it);
	}
	
	// find all users violating h constraint
	set<User*> B;
	for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
		User* u = *it;
		set<User*> out_h;
		u->get_users_out_of_h_hop(U, h, out_h);
		if(out_h.size() > 0){
			B.insert(u);
			B.insert(out_h.begin(), out_h.end());
		}		
	}
		
	// Expand
	set<User*> Z;
	set<User*> to_remove;
	double max = 0.0;
	User* selected;
	set_difference(all.begin(), all.end(), U.begin(), U.end(), inserter(Z, Z.begin()));	
	
	for(set<User*>::iterator it = Z.begin(); it != Z.end(); it++){
		User* u = *it;
		double value = u->get_sum_of_all_potential_weights();
		if(value == 0){
			to_remove.insert(u);
		}
		else if(!u->is_all_U_in_h_hop(U, h)){
			to_remove.insert(u);			
		}
		else{
			ret.insert(u);
			double obj = compute_social_obj(ret);
			ret.erase(u);
			if(obj - least_obj > max){
				max = obj - least_obj;
				selected = u;
			} 
		}		
	}
	set<User*> leftZ;
	set_difference(Z.begin(), Z.end(), to_remove.begin(), to_remove.end(), inserter(leftZ, leftZ.begin()));
	Z = leftZ;
	leftZ.clear();
	to_remove.clear();
	
	
	while(max > 0){		
		Z.erase(selected);
		ret.insert(selected);
		least_obj += max;
		set<User*> selected_h_hop = selected->get_h_hop_neighborhood(h);
		set_intersection(Z.begin(), Z.end(), selected_h_hop.begin(), selected_h_hop.end(), inserter(leftZ, leftZ.begin()));
		Z = leftZ;
		leftZ.clear();
		max = 0.0;
		
		for(set<User*>::iterator it = Z.begin(); it != Z.end(); it++){
			User* u = *it;
			ret.insert(u);
			double obj = compute_social_obj(ret);
			ret.erase(u);
			if(obj - least_obj > max){
				max = obj - least_obj;
				selected = u;
			}  
		}
	}
	Z.clear();
	to_remove.clear();
	
	
	// Shrink
	double min = least_obj;
	for(set<User*>::iterator it = B.begin(); it != B.end(); it++){
		User* u = *it;
		double value = u->get_sum_of_potential_weights(ret);
		if(value < min){
			min = value;
			selected = u;
		}
	}
	
	while(min < least_obj && ret.size() > n && B.size() > 0){
		least_obj = (least_obj * ret.size() - min)/(ret.size() -1 );
		ret.erase(selected);
		B.erase(selected);	
		min = least_obj;
		
		for(set<User*>::iterator it = B.begin(); it != B.end(); it++){
			User* u = *it;
			double value = u->get_sum_of_potential_weights(ret);
			if(value < min){
				min = value;
				selected = u;
			}
		}
	}	
	
	return ret;	
}


void write_out(string filename, set<User*> U, double opt, double time){
	// format: opt, time, U, O
	fstream ff;
	ff.open(filename, ios::out);
	ff << opt << "\t" << time << "\t";
	bool first = true;
	for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
		if(first){
			first = false;			
		}
		else{
			ff << ",";
		}
		ff << (*it)->id;			
	}
	ff << endl;
	ff.close();
}