#include<cstdlib>
#include<iostream>
#include<vector>
//#include<fstream>
#include<string>
#include<sstream>
#include<set>
#include<utility>
#include<vector>	
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

void thread_preliminary(list<User*> todo, Graph* g);
Solution* topic_oriented(Graph* g, set<User*> H_v, int n, int m);
Solution* post_processing(Graph* g, Solution* s, int n, int m, int h, bool byb);
Solution* mission(User* v, Graph* g, int n, int m, int h, int thread_no, int counter, double max, bool pruning);
Solution* thread_mission(list<User*> todo, Graph* g, int n, int m, int h, int thread_no, bool pruning);
void write_out(string filename, set<User*> U, set<Item*> O, double opt, double time, double fea, int pruned);

int main(int argc, char* argv[]){
	// network, n, m, h
	string network = "test";
	int n = 1;
	int m = 1;
	int h = 1;
	bool pruning = true;
	bool pp = true;
	bool byb = false;
	int n_thread = 1;
	
	if(argc >= 8){
		network = argv[1];
		n = std::stoi(argv[2], NULL);
		m = std::stoi(argv[3], NULL);
		h = std::stoi(argv[4], NULL);
		pruning = std::stoi(argv[5], NULL) == 1 ? true : false; 
		pp = std::stoi(argv[6], NULL) == 1 ? true : false;
		byb = std::stoi(argv[7], NULL) == 1 ? true : false;		
		if(argc >= 9){
			n_thread = std::stoi(argv[8], NULL);
		}
	}
	
	Graph* g = read(network);
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
			(*it)->get_all_topic_engagement(g->query_topics);
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
			f.push_back(std::async(std::launch::async, thread_preliminary, todos.at(i), g));
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
	set<Item*> opt_O;
	int prune_count = 0;
	
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		User* v = *it;
		if(n_thread == 1){
			Solution* s = mission(v, g, n, m, h, 1, counter, opt, pruning);
			if(s == NULL){
				prune_count++;
			}
			if(s != NULL && opt < s->opt){
				opt = s->opt;
				opt_U = s->U;
				opt_O = s->O;
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
			f.push_back(std::async(std::launch::async, thread_mission, todos.at(i), g, n, m, h, i, pruning));
		}		
		for(list<future<Solution*>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			Solution* s = (*f_it).get();
			if(s != NULL && opt < s->opt){
				opt = s->opt;
				opt_U = s->U;
				opt_O = s->O;
				prune_count += s->pruned;
			}					
		}
	}
	
	cout << "Before post processing: " << opt << endl;
	
	if(pp){
		cout << "Start post processing" << endl;
		Solution* ret = new Solution(opt, opt_U, opt_O);
		ret = post_processing(g, ret, n, m, h, byb);
		opt_U = ret->U;
		opt_O = ret->O;
		opt = ret->opt;
	}
	
	clock_t end = clock();	
	
	// output
	stringstream ss;
	ss << network << "_" << n << "_" << m << "_" << h << "_" << argv[5] << "_" << argv[6] << "_" << argv[7] << ".out";
	double t_all = double(end-start)/double(CLOCKS_PER_SEC);
	double fea = compute_feasibility(opt_U, h, n_thread);
	write_out(ss.str(), opt_U, opt_O, opt, t_all, fea, prune_count);
}

void thread_preliminary(list<User*> todo, Graph* g){
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		(*it)->get_all_topic_engagement(g->query_topics);
		(*it)->get_sum_of_all_potential_weights();
	}
}

Solution* thread_mission(list<User*> todo, Graph* g, int n, int m, int h, int thread_no, bool pruning){
	double obj = 0.0;
	set<User*> U;
	set<Item*> O;
	int prune_count = 0;
	
	int counter = 0;
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		Solution* s = mission(*it, g, n, m, h, thread_no, counter++, obj, pruning);
		if(s == NULL){
			prune_count++;
		}
		if(s != NULL && obj < s->opt){
			obj = s->opt;
			U = s->U;
			O = s->O;
		}
	}
	
	Solution* ret = new Solution(obj, U, O, prune_count);
	return ret; 
}

Solution* mission(User* v, Graph* g, int n, int m, int h, int thread_no, int counter, double max, bool pruning){
	counter++;
		
	cout << "[" << thread_no << "] " << counter << ": " << v->id << " starts" << endl;
	set<User*> H_v = v->get_h_hop_neighborhood(h);
	
	double MaxGF_upper_bound = 0.0;
	map<User*, double> tau;
		
	if(H_v.size() < n){
		return NULL;
	}
	
	if(pruning){		
		double max_social = 0.0;
		double total_topic = 0.0;
		double MaxGF_social = 0.0;
		double MaxGF_topic = 0.0;	
		vector<double> top_n;	
		for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
			// social
			double v_social = (*it)->get_sum_of_potential_weights(H_v);
			tau[*it] = v_social;
			if(v_social >= max_social){
				max_social = v_social;
			}

			// topic 
			double v_topic = (*it)->get_all_topic_engagement(g->query_topics);
			total_topic += v_topic;
			
			// For further prune MaxGF
			if(v_social != 0){
				// social: find top-n v_social				
				if(top_n.size() < n){
					top_n.push_back(v_social);
					if(top_n.size() == n){
						sort_heap(top_n.begin(), top_n.end(), min_double_heap());
					}
				}
				else if(top_n.front() <= v_social){
						pop_heap(top_n.begin(), top_n.end(), min_double_heap());
						top_n.pop_back();
						top_n.push_back(v_social);
						push_heap(top_n.begin(), top_n.end(), min_double_heap());
						sort_heap(top_n.begin(), top_n.end(), min_double_heap());
				}
				else{
					// top_n.size() >=n but mypair is less than the min of top_n, so do nothing
				}
				
				// topic: sum all users with v_social > 0
				MaxGF_topic += v_topic;
			} 
		}
		if(max_social/2 + total_topic/g->query_topics.size() <= max){
			return NULL;
		}
		for(int i=0; i<top_n.size(); i++){
			MaxGF_social += top_n[i];
		}
		MaxGF_upper_bound = MaxGF_social/(2*n) + MaxGF_topic/g->query_topics.size();
	}
	
	cout << "[" << thread_no << "] " << counter << ": " << v->id << " pruning finished" << endl;
	
						 
	// Step 2: Topic-oriented strategy
	Solution* s_T = topic_oriented(g, H_v, n, m);
//	cout << "[" << thread_no << "] " << counter << ": " << v->id << " Step 2 finished" << endl;

	// Step 1: Social-oriented strategy (MaxGF)
	/*if(pruning && (MaxGF_upper_bound <= max || MaxGF_upper_bound <= s_T->opt)){
		// If pruning and we know MaxGF is not large enough, return s_T directly
		cout << "new prune" << endl;
		return s_T;
	}*/
	set<User*> U_S = MaxGF(H_v, n, tau);
	set<Item*> O_S = g->opt_itemset(U_S, m);
	double obj_S = compute_obj(U_S, O_S, g->query_topics, tau);
//	cout << "[" << thread_no << "] " << counter << ": " << v->id << " Step 1 finished" << endl;


	Solution* ret;	
	if(s_T->opt > obj_S){
		ret = s_T;
		cout << "[" << thread_no << "] " << counter << ": " << v->id << " Step 2 wins " << ret->opt << endl;
	}
	else{
		ret = new Solution(obj_S, U_S, O_S);
		cout << "[" << thread_no << "] " << counter << ": " << v->id << " Step 1 wins " << ret->opt << endl;
	}
	
	 
	
	return ret;
}


Solution* topic_oriented(Graph* g, set<User*> H_v, int n, int m){
	set<User*> U;
	set<Item*> O = g->opt_itemset(H_v, m);
//	double max = compute_obj(H_v, O, g->query_topics);
	double max = 0;
//	cout << "max = " << max << endl;
	
	set<User*> S;
	double social_weights = 0.0;
	double topic_obj = 0.0;
	for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
		User* u = *it;
		S.insert(u);
		social_weights += u->get_sum_of_potential_weights(S);
		topic_obj += u->get_all_topic_engagement(O, g->query_topics);
//		cout << u->id << ": " << social_weights/S.size() << " + " << topic_obj << " = " << social_weights/S.size()+topic_obj << endl;
		if(S.size() >= n){
			double obj = social_weights/S.size() + topic_obj;
			if(obj >= max){
				max = obj;
				U = S;
			}
		}	 
	}
	
	if(U.size() != H_v.size()){
		O = g->opt_itemset(S, m);
		max = compute_obj(U, O, g->query_topics);		
	}
	
	Solution* ret = new Solution(max, U, O);
}

Solution* post_processing(Graph* g, Solution* s, int n, int m, int h, bool byb){
	set<User*> all;
	set<User*> ret_U(s->U);
	set<Item*> ret_O(s->O);
	double least_obj = s->opt;
	
	for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
		all.insert(*it);
	}

	bool loop = false;
	double max;
	User* selected;	
	
	// find all users violating h constraint
	set<User*> B;
	for(set<User*>::iterator it = ret_U.begin(); it != ret_U.end(); it++){
		User* u = *it;
		set<User*> out_h;
		u->get_users_out_of_h_hop(ret_U, h, out_h);
		if(out_h.size() > 0){
			B.insert(u);
			B.insert(out_h.begin(), out_h.end());
		}		
	}
	
	// Expand
	set<User*> Z;
	set<User*> to_remove;
	set_difference(all.begin(), all.end(), ret_U.begin(), ret_U.end(), inserter(Z, Z.begin()));
	
	for(set<User*>::iterator it = Z.begin(); it != Z.end(); it++){
		User* u = *it;
		double value = u->get_sum_of_all_potential_weights();
		if(value == 0){
			to_remove.insert(u);
		}
		else if(!u->is_all_U_in_h_hop(ret_U, h)){
			to_remove.insert(u);			
		}
		else{
			// do nothing
		}		
	}
	set<User*> leftZ;
	set_difference(Z.begin(), Z.end(), to_remove.begin(), to_remove.end(), inserter(leftZ, leftZ.begin()));
	Z = leftZ;
	leftZ.clear();
	to_remove.clear();
	
	do{
		max = 0.0;
		for(set<User*>::iterator it = Z.begin(); it != Z.end(); it++){
			User* u = *it;
			ret_U.insert(u);
			double obj = compute_obj(ret_U, ret_O, g->query_topics);
			ret_U.erase(u);
			if(obj - least_obj > max){
				max = obj - least_obj;
				selected = u;
				loop = true;
			} 
		}	
		
		while(max > 0){		
			Z.erase(selected);
			ret_U.insert(selected);
			least_obj += max;
			set<User*> selected_h_hop = selected->get_h_hop_neighborhood(h);
			set_intersection(Z.begin(), Z.end(), selected_h_hop.begin(), selected_h_hop.end(), inserter(leftZ, leftZ.begin()));
			Z = leftZ;
			leftZ.clear();
			max = 0.0;
			
			for(set<User*>::iterator it = Z.begin(); it != Z.end(); it++){
				User* u = *it;
				ret_U.insert(u);
				double obj = compute_obj(ret_U, ret_O, g->query_topics);
				ret_U.erase(u);
				if(obj - least_obj > max){
					max = obj - least_obj;
					selected = u;
				}  
			}
		}
		ret_O = g->opt_itemset(ret_U, m);
	} while(loop && byb);

 	double social_obj = compute_social_obj(ret_U);
	double topic_obj = compute_topic_obj(ret_U, ret_O, g->query_topics);
	least_obj = compute_obj(ret_U, ret_O, g->query_topics);
	Z.clear();
	to_remove.clear();
	loop = false;	
	
	// Shrink
	do{
		max = 0.0;
		double social_max = 0.0;
		double topic_max = 0.0;
		map<User*, double> PTE;       
		for(set<User*>::iterator it = B.begin(); it != B.end(); it++){
			User* u = *it;
			double social_value = u->get_sum_of_potential_weights(ret_U);
			double topic_value = u->get_all_topic_engagement(ret_O, g->query_topics);
			PTE[u] = topic_value;
			double social_diff = (social_obj-social_value)/(ret_U.size()-1);  
			double topic_diff = -(topic_obj-topic_value);
			if(social_diff+topic_diff > max){
				max = social_diff+topic_diff;
				social_max = social_diff;
				topic_max = topic_diff;
				selected = u;
			}
/*			ret_U.erase(u);
			double obj = compute_obj(ret_U, ret_O, g->query_topics);
			ret_U.insert(u);
			if(obj - least_obj > max){
				max = obj - least_obj;
				selected = u;
			}*/ 
		}
		
		while(max > 0 && ret_U.size() > n && B.size() > 0){
			social_obj += social_max;
			topic_obj += topic_max;
			ret_U.erase(selected);
			B.erase(selected);	
			max = 0.0;
			social_max = 0.0;
			topic_max = 0.0; 
			
			for(set<User*>::iterator it = B.begin(); it != B.end(); it++){
				User* u = *it;
				double social_value = u->get_sum_of_potential_weights(ret_U);
				double topic_value = PTE[u];
				double social_diff = (social_obj-social_value)/(ret_U.size()-1);  
				double topic_diff = -(topic_obj-topic_value);
				if(social_diff+topic_diff > max){
					max = social_diff+topic_diff;
					social_max = social_diff;
					topic_max = topic_diff;
					selected = u;
				}
/*				ret_U.erase(u);
				double obj = compute_obj(ret_U, ret_O, g->query_topics);
				ret_U.insert(u);
				if(obj - least_obj > max){
					max = obj - least_obj;
					selected = u;
				}*/ 
			}
		}
		ret_O = g->opt_itemset(ret_U, m);
		social_obj = compute_social_obj(ret_U);
		topic_obj = compute_topic_obj(ret_U, ret_O, g->query_topics);
//		least_obj = compute_obj(ret_U, ret_O, g->query_topics);
	} while(loop && byb);	
	
	Solution* ret = new Solution(social_obj+topic_obj, ret_U, ret_O);
	return ret;	
}


void write_out(string filename, set<User*> U, set<Item*> O, double opt, double time, double fea, int pruned){
	// format: opt, time, feasibility, U, O
	fstream ff;
	ff.open(filename, ios::out);
	ff << opt << "\t" << time << "\t" << fea << "\t" << pruned << "\t";
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