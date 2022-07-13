#include<cstdlib>
#include<iostream>
#include<vector>
//#include<fstream>
#include<string>
#include<sstream>
#include<set>
#include<tuple>
#include<utility>
//#include<ctime>
#include<algorithm>
#include<future>
#include<random>
#include<functional>
//#include<map>
//#include<utility>
//#include<queue>
//#include<cmath>
//#include<pthread.h>
//#include<unistd.h>
#include "Graph.h"

using namespace std;

void thread_user_relevance_mission(list<User*> todo, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk, string network);
void user_relevance_mission(User* v, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk, string network);
double compute_relevance(User* v, Item* i, Topic* t, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk);
int random_walk(User* v, Item* i, Topic* t, double alpha, double beta, double gamma, double lambda, int l_walk);
int tuple_type(tuple<User*, Item*, Topic*> tp);
void print_tuple(tuple<User*, Item*, Topic*> tp);

int main(int argc, char* argv[]){
	// network, alpha, beta, gamma, lambda, l_walk, n_walk
	string network = "test";
	double alpha = 0.5;
	double beta = 0.5;
	double gamma = 0.5;
	double lambda = 0.5;
	int l_walk = 5;
	int n_walk = 10;
	int n_thread = 1;
	
	if(argc >= 8){
		network = argv[1];
		alpha = std::stod(argv[2], NULL);
		beta = std::stod(argv[3], NULL);
		gamma = std::stod(argv[4], NULL);
		lambda = std::stod(argv[5], NULL);
		l_walk = std::stoi(argv[6], NULL);
		n_walk = std::stoi(argv[7], NULL);
		if(argc >= 9){
			n_thread = std::stoi(argv[8], NULL);
		}
	}
	
	Graph* g = read_relation(network);
	cout << "Finish reading" << endl;
	
	int counter = 0;
	vector<list<User*>> todos;
	for(int i=0; i<n_thread; i++){
		list<User*> todo;
		todos.push_back(todo);
	}
	
	if(n_thread == 1){
		srand(time(NULL));
		for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
			user_relevance_mission(*it, alpha, beta, gamma, lambda, l_walk, n_walk, network);
		} 
	}
	else{
		for(list<User*>::iterator it = g->users.begin(); it != g->users.end(); it++){
			int no = counter++ % n_thread;
			todos.at(no).push_back(*it);
		}
		list<future<void>> f;
		for(int i=0; i<todos.size(); i++){
			 f.push_back(std::async(std::launch::async, thread_user_relevance_mission, todos.at(i), alpha, beta, gamma, lambda, l_walk, n_walk, network));
		}
		for(list<future<void>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			(*f_it).get();
		}
	}
}

void thread_user_relevance_mission(list<User*> todo, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk, string network){
	for(list<User*>::iterator it = todo.begin(); it != todo.end(); it++){
		user_relevance_mission(*it, alpha, beta, gamma, lambda, l_walk, n_walk, network);
	}
}

void user_relevance_mission(User* v, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk, string network){
	stringstream ss;
	for(map<Item*, PreferenceEdge*>::iterator it = v->preference.begin(); it != v->preference.end(); it++){
		Item* i = it->first;
		for(map<Topic*, RelationEdge*>::iterator t_it = i->relation.begin(); t_it != i->relation.end(); t_it++){
			Topic* t = t_it->first;
			ss << v->id << " " << i->id << " " << t->id << " " << compute_relevance(v, i, t, alpha, beta, gamma, lambda, l_walk, n_walk) << endl; 
		}
	}
	
	stringstream filename;
	filename << network << "_" << v->id << ".rel";
	fstream ff;
	ff.open(filename.str(), ios::out);
	ff << ss.str();
	ff.close();
}

double compute_relevance(User* v, Item* i, Topic* t, double alpha, double beta, double gamma, double lambda, int l_walk, int n_walk){
	double sum = 0.0;
	for(int j=0; j<n_walk; j++){ 
		sum += random_walk(v, i, t, alpha, beta, gamma, lambda, l_walk);
	}
	return sum/(double)n_walk;
}

int random_walk(User* v, Item* i, Topic* t, double alpha, double beta, double gamma, double lambda, int l_walk){
	double to_SN = v->preference[i]->weight;
	double p_start = rand() / (RAND_MAX + 1.0);
	tuple<User*, Item*, Topic*> cur(v, NULL, NULL);
	tuple<User*, Item*, Topic*> pre(NULL, i, NULL);
	
	if(p_start >= to_SN){		
		int index = (double)(rand() / (RAND_MAX + 1.0)) * i->relation.size();
		map<Topic*, RelationEdge*>::iterator it = i->relation.begin();
		advance(it, index);
		if(it->first == t){
			return 1;
		}
		tuple<User*, Item*, Topic*> tmp(NULL, NULL, it->first);
		cur = tmp;
	}
	
	int step = 1;
	while(step < l_walk){
		if(tuple_type(cur) == 0){
			// cur is a user
			User* user = get<0>(cur);
			bool stay_in_SN = false;
			if(tuple_type(pre) == 0){
				// pre is also a user
				double p_alpha = rand() / (RAND_MAX + 1.0);
				if(p_alpha < alpha){
					stay_in_SN = true;
				}				
			}
			else if (tuple_type(pre) == 1){
				// pre is an item
				double p_beta = rand() / (RAND_MAX + 1.0);
				if(p_beta < beta){
					stay_in_SN = true;
				}
			}
			else{
				// pre should not be a topic
			}
			
			if(stay_in_SN){
				// go to another user
				if(user->friends.size() == 0){
					break;
				}
				int index = (double)(rand() / (RAND_MAX + 1.0)) * user->friends.size();
				map<User*, FriendEdge*>::iterator it = user->friends.begin();
				advance(it, index);
				pre = cur;
				tuple<User*, Item*, Topic*> tmp(it->first, NULL, NULL);
				cur = tmp;
			}
			else{
				// go to an item
				if(user->preference.size() == 0){
					break;
				}
				int index = (double)(rand() / (RAND_MAX + 1.0)) * user->preference.size();
				map<Item*, PreferenceEdge*>::iterator it = user->preference.begin();
				advance(it, index);
				pre = cur;
				tuple<User*, Item*, Topic*> tmp(NULL, it->first, NULL);
				cur = tmp;
			}
		}
		else if(tuple_type(cur) == 1){
			// cur is an item
			Item* item = get<1>(cur); 
			bool stay_in_KG = false;
			if(tuple_type(pre) == 0){
				// pre is a user
				double p_gamma = rand() / (RAND_MAX + 1.0);
				if(p_gamma < gamma){
					stay_in_KG = true;
				}
			}
			else if(tuple_type(pre) == 2){
				// pre is a topic
				double p_lambda = rand() / (RAND_MAX + 1.0);
				if(p_lambda < lambda){
					stay_in_KG = true;
				}
			}
			else{
				// pre should not be an item
			}
			
			if(stay_in_KG){
				// go to a topic
				if(item->relation.size() == 0){
					break;
				}
				int index = (double)(rand() / (RAND_MAX + 1.0)) * item->relation.size();
				map<Topic*, RelationEdge*>::iterator it = item->relation.begin();
				advance(it, index);
				if(it->first == t){
					return 1;
				}
				pre = cur;
				tuple<User*, Item*, Topic*> tmp(NULL, NULL, it->first);
				cur = tmp;				
			}
			else{
				// go to a user
				if(item->preference.size() == 0){
					break;
				}
				int index = (double)(rand() / (RAND_MAX + 1.0)) * item->preference.size();
				map<User*, PreferenceEdge*>::iterator it = item->preference.begin();
				advance(it, index);
				pre = cur;
				tuple<User*, Item*, Topic*> tmp(it->first, NULL, NULL);
				cur = tmp;
			}
		}
		else{
			// cur is a topic, go to an item
			Topic* topic = get<2>(cur);
			if(topic->relation.size() == 0){
				break;
			}
			int index = (double)(rand() / (RAND_MAX + 1.0)) * topic->relation.size();
			map<Item*, RelationEdge*>::iterator it = topic->relation.begin();
			advance(it, index);
			pre = cur;
			tuple<User*, Item*, Topic*> tmp(NULL, it->first, NULL);
			cur = tmp;
		}
		step++;
	}
	return 0;
}

int tuple_type(tuple<User*, Item*, Topic*> tp){
	if(get<0>(tp) != NULL){
		return 0;
	}
	if(get<1>(tp) != NULL){
		return 1;
	}
	if(get<2>(tp) != NULL){
		return 2;
	}
	return -1;
}

void print_tuple(tuple<User*, Item*, Topic*> tp){
	int tt = tuple_type(tp);
	if(tt == 0){
		User* v = get<0>(tp);
		cout << v->id << "(" << tt << ")" << endl;
	}
	else if(tt == 1){
		Item* i = get<1>(tp);
		cout << i->id << "(" << tt << ")" << endl;
	}
	else{
		Topic* t = get<2>(tp);
		cout << t->id << "(" << tt << ")" << endl;
	}
}