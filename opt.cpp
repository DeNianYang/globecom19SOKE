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

void thread_preliminary(list<User*> todo, Graph* g, int h);
Solution* thread_user_groups_dfs(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no);
Solution* thread_user_groups_bfs(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no);
Solution* thread_user_groups_bfs_new(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no, set<User*> zero);
void write_out(string filename, set<User*> U, set<Item*> O, double opt, double time);

int main(int argc, char* argv[]){
	// network, n, m, h, speficied
	string network = "test";
	int n = 1;
	int m = 1;
	int h = 1;
	int sp = -1;
	int n_thread = 1;
	
	if(argc >= 5){
		network = argv[1];
		n = std::stoi(argv[2], NULL);
		m = std::stoi(argv[3], NULL);
		h = std::stoi(argv[4], NULL);
		
		if(argc >= 6){
			sp = std::stoi(argv[5], NULL);

			if(argc >= 7){
				n_thread = std::stoi(argv[6], NULL);
			}
		}
		
	}
	
	Graph* g = read(network);
	cout << "Finish reading" << endl;
		
	
	clock_t start = clock();
	
	double opt = 0.0;
	set<User*> opt_U;
	set<Item*> opt_O;
	
	int counter = 0;
	vector<list<User*>> todos;
	for(int i=0; i<n_thread; i++){
		list<User*> todo;
		todos.push_back(todo);
	}
	
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		int no = counter++ % n_thread;
		todos.at(no).push_back(*it);
	}
	
	
	list<future<void>> pre;
	for(int i=0; i<todos.size(); i++){
		pre.push_back(std::async(std::launch::async, thread_preliminary, todos.at(i), g, h));
	}
	for(list<future<void>>::iterator f_it = pre.begin(); f_it != pre.end(); f_it++){
		(*f_it).get();
	}
	g->sort_users_by_hop();
	pre.clear();
	cout << "Preliminary done" << endl;
	
	set<User*> zero;
	int no_tau = 0;
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		if((*it)->get_all_topic_engagement(g->query_topics) == 0 && (*it)->get_sum_of_all_potential_weights() == 0){
			zero.insert(*it);
		}
		if((*it)->get_sum_of_all_potential_weights() == 0){
			no_tau ++;
		}
	}
	cout << no_tau << endl;
	
	
	// init future-related variables
	for(int i=0; i<n_thread; i++){			
		todos.at(i).clear();
	}
	counter = 0;
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		int no = counter++ % n_thread;
		if(sp == -1 || counter >= sp-1){
			todos.at(no).push_back(*it);
		}		
	}
	
	list<future<Solution*>> f;
	for(int i=0; i<todos.size(); i++){
//		f.push_back(std::async(std::launch::async, thread_user_groups_dfs, todos.at(i), g, n, m, h, n_thread, i));
		f.push_back(std::async(std::launch::async, thread_user_groups_bfs_new, todos.at(i), g, n, m, h, n_thread, i, zero));
	}
	for(list<future<Solution*>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
		Solution* s = (*f_it).get();
		if(s != NULL && opt < s->opt){
			opt = s->opt;
			opt_U = s->U;
			opt_O = s->O;
		}
	}
	
	clock_t end = clock();

	// output
	stringstream ss;
	ss << network << "_" << n << "_" << m << "_" << h << "_opt.out";
	double t_all = double(end-start)/double(CLOCKS_PER_SEC);
	write_out(ss.str(), opt_U, opt_O, opt, t_all);
}

void thread_preliminary(list<User*> todo, Graph* g, int h){
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		(*it)->get_h_hop_count(h);
		(*it)->get_all_topic_engagement(g->query_topics);
		(*it)->get_sum_of_all_potential_weights();
	}
}

// by recusrive
Solution* thread_user_groups_dfs(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no){
	double obj = 0.0;
	set<User*> U;
	set<Item*> O;
	
	int counter = 0;
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		set<User*> init;
		init.insert(*it);
		int offset = n_thread*(counter++)+thread_no+1;
		Solution* s = g->get_opt_at_least_n(init, n, m, h, offset); 
		if(s != NULL && obj < s->opt){
			obj = s->opt;
			U = s->U;
			O = s->O;
		}
	}
	
	Solution* ret = new Solution(obj, U, O);
	return ret; 
}

// by iterations
Solution* thread_user_groups_bfs(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no){
	double obj = 0.0;
	set<User*> U;
	set<Item*> O;
	
	int counter = 0;
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		set<User*> init_set;
		init_set.insert(*it);
		int offset = n_thread*(counter++)+thread_no+1;
		UserGroupGrow* init_ugg = new UserGroupGrow(init_set, offset);
		list<UserGroupGrow*> init;
		init.push_back(init_ugg);

		
		Solution* s = g->get_opt_at_least_n(init, n, m, h); 
		if(s != NULL && obj < s->opt){
			obj = s->opt;
			U = s->U;
			O = s->O;
		}
		
		cout << "[" << thread_no << "] " << counter << " ended, " << todo.size()-counter << " tasks left" << endl;
	}
	
	Solution* ret = new Solution(obj, U, O);
	return ret; 
}

// by iterations (new with pruning)
Solution* thread_user_groups_bfs_new(list<User*> todo, Graph* g, int n, int m, int h, int n_thread, int thread_no, set<User*> zero){
	double obj = 0.0;
	set<User*> U;
	set<Item*> O;
	
	int counter = 0;
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		clock_t start = clock();
		
		set<User*> pool;
		bool found = false;
		int sn = 0;
		for(list<User*>::iterator u_it = g->users.begin(); u_it != g->users.end(); u_it++){			
			if(!found){				
				if(*it == *u_it){
					found = true;
				}
				else{
					sn++;
				}
			}
			else{
				pool.insert(*u_it);
			}
		}
	
		if(zero.find(*it) == zero.end()){
			set<User*> init_set;
			init_set.insert(*it);
			
			set<User*> h_hop;
			(*it)->get_users_in_h_hop(pool, h, h_hop);
			set<User*> init_pool;
			set_difference(h_hop.begin(), h_hop.end(), zero.begin(), zero.end(), inserter(init_pool, init_pool.begin()));
			h_hop.clear();
			pool.clear();
			
			map<Item*, double> init_pte;
			for(map<Item*, PreferenceEdge*>::iterator up_it = (*it)->preference.begin(); up_it != (*it)->preference.end(); up_it++){
				double sum = 0.0;
				if((*it)->relevance.find(up_it->first) != (*it)->relevance.end()){
					double w = up_it->second->weight;
					for(list<int>::iterator utr_it = g->query_topics.begin(); utr_it != g->query_topics.end(); utr_it++){
						sum += w * (*it)->relevance[up_it->first][*utr_it];
					}					
				}
				if(sum != 0){
					init_pte[up_it->first] = sum;
				}
			}		
			UserGroupGrow* init_ugg = new UserGroupGrow(init_set, init_pool, init_pte, 0);
			list<UserGroupGrow*> init;
			init.push_back(init_ugg);
			
			init_set.clear();
			init_pool.clear();
			init_pte.clear();
	
			
			Solution* s = g->get_opt_at_least_n_new(init, n, m, h, thread_no); 
			if(s != NULL && obj < s->opt){
				obj = s->opt;
				U = s->U;
				O = s->O;
			}
		}
		
		clock_t end = clock();
		
		stringstream ss;
		ss << n << "_" << m << "_" << h << "_" << sn << "_opt.out";
		double t_all = double(end-start)/double(CLOCKS_PER_SEC);
		write_out(ss.str(), U, O, obj, t_all);
		ss.clear();
		
		cout << "[" << thread_no << "] " << ++counter << " ended, " << todo.size()-counter << " tasks left" << endl;
	}
	
	Solution* ret = new Solution(obj, U, O);
	U.clear();
	O.clear();
	return ret; 
}

void write_out(string filename, set<User*> U, set<Item*> O, double opt, double time){
	// format: opt, time, U, O
	fstream ff;
	ff.open(filename, ios::out);
	ff << opt << "\t" << time << "\t" ;
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
	ff << "\t";
	first = true;
	for(set<Item*>::iterator it = O.begin(); it != O.end(); it++){
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