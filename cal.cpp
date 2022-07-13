#include<cstdlib>
#include<iostream>
//#include<vector>
//#include<fstream>
#include<string>
#include<sstream>
#include<set>
#include<utility>
//#include<ctime>
#include<algorithm>
#include<future>
//#include<map>
//#include<utility>
//#include<queue>
//#include<cmath>
//#include<pthread.h>
//#include<unistd.h>
#include "Graph.h"

using namespace std;

void baseline_mission(Baseline* b, Graph* g, string network);
void thread_baseline_mission(list<Baseline*> todo, Graph* g, string network, int thread_no);

int main(int argc, char* argv[]){
	// network
	string network = "test";
	int n_thread = 1;
	
	if(argc >= 2){
		network = argv[1];
		if(argc >= 3){
			n_thread = std::stoi(argv[2], NULL);
		}
	}
	
	pair<Graph*, list<Baseline*>> results = read_baselines(network);
	Graph* g = results.first;
	list<Baseline*> baselines = results.second;
	cout << "Finish reading" << endl;
	
	clock_t start = clock();
	
	int counter = 0;
	vector<list<Baseline*>> todos;
	for(int i=0; i<n_thread; i++){
		list<Baseline*> todo;
		todos.push_back(todo);
	}
	
	if(n_thread == 1){
		stringstream simple;
		for(list<Baseline*>::iterator it = baselines.begin(); it != baselines.end(); it++){
			Baseline* b = *it;
			baseline_mission(b, g, network);
			simple << b->to_string();
		}
		stringstream filename;
		filename << network << "_simple.out";	
		fstream ff;
		ff.open(filename.str(), ios::out);
		ff << simple.str();
		ff.close(); 
	}
	else{
		for(list<Baseline*>::iterator it = baselines.begin(); it != baselines.end(); it++){
			int no = counter++ % n_thread;
			todos.at(no).push_back(*it);
		}
		list<future<void>> f;
		for(int i=0; i<todos.size(); i++){
			 f.push_back(std::async(std::launch::async, thread_baseline_mission, todos.at(i), g, network, i));
		}
		for(list<future<void>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			(*f_it).get();
		}
	}
}

void baseline_mission(Baseline* b, Graph* g, string network){
	cout << "O" << endl;
	if(b->O.size() == 0){
		set<Item*> O = g->opt_itemset(b->U, b->m);
		b->set_O(O);
	}
	
	cout << "obj" << endl;
	
	double se = compute_social_obj(b->U);
	double pte = compute_topic_obj(b->U, b->O, g->query_topics);
	double obj = se+pte;
	b->set_se(se);
	b->set_pte(pte);
	b->set_obj(obj);

	cout << "feasibility" << endl;
		
	double fea = compute_feasibility(b->U, b->h, 1);
	b->set_feasibility(fea);
	
	cout << "finish" << endl;
	
	stringstream filename;
	filename << network << "_" << b->to_name();
	fstream ff;
	ff.open(filename.str(), ios::out);
	ff << b->to_full_string();
	ff.close();
	
	cout << "===" << endl;	
}

void thread_baseline_mission(list<Baseline*> todo, Graph* g, string network, int thread_no){
	stringstream simple;
	for(list<Baseline*>::iterator it = todo.begin(); it != todo.end(); it++){
		Baseline* b = *it;
		baseline_mission(b, g, network);
		simple << b->to_string();
	}
	
	stringstream filename;
	filename << network << "_" << thread_no << "_simple.out";	
	fstream ff;
	ff.open(filename.str(), ios::out);
	ff << simple.str();
	ff.close(); 
}


