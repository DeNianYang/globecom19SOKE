#include<cstdlib>
#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<ctime>
#include<sstream>
#include<algorithm>
#include<map>
#include<list>
#include<set>
#include<iterator>
//#include<utility>
//#include<queue>
using namespace std;

class User;
class Item;
class Topic;
class FriendEdge;
class PotentialEdge;
class PreferenceEdge;
class RelationEdge;
class Solution;
class TopItem;
class UserGroupGrow;
class Graph;
class Baseline;


struct min_heap{
	bool operator()(const pair<double, Item*>& a,const pair<double, Item*>& b) const{
		return a <= b;
	}
};

struct min_double_heap{
	bool operator()(const double& a,const double& b) const{
		return a <= b;
	}
};

class FriendEdge{
	public:
		FriendEdge(User* a, User* b){
			u1 = a;
			u2 = b;
		}
		
		User* u1;
		User* u2;		
};

class PotentialEdge{
	public:
		PotentialEdge(User* a, User* b, double w){
			u1 = a;
			u2 = b;
			weight = w;
		}
		
		User* u1;
		User* u2;
		double weight;						
};

class PreferenceEdge{
	public:
		PreferenceEdge(User* a, Item* b, double w){
			u = a;
			i = b;
			weight = w;
		}
		
		User* u;
		Item* i;
		double weight;						
};

class RelationEdge{
	public:
		RelationEdge(Item* a, Topic* b){
			i = a;
			t = b;
		}
		
		Item* i;
		Topic* t;					
};


class User{
	public:
		User(string s){
			id = s;
			type_u_i_t = 0;
		}		

		string id;
		int type_u_i_t;
		map<User*, FriendEdge*> friends;
		map<User*, PotentialEdge*> potential;
		map<Item*, PreferenceEdge*> preference;
		map<Item*, vector<double>> relevance;
		double sum_of_all_potential_weights = -1;
		double at_least_preference = -1;
		double all_topic_engagement = -1;
		double h_hop_count = -1;
		bool h_hop_searched = false;
		set<User*> h_hop;
		
		void add_relevance(Item* i, int topic_index, double r){
			if(relevance.find(i) == relevance.end()){
				// default number of topics = 9000
				vector<double> pr(9000, 0);
				pr[topic_index] = r;
				relevance[i] = pr;
			}
			else{
				vector<double> pr = relevance[i];
				pr[topic_index] = r;
			}
		}
		
		double get_h_hop_count(int h){
			if(h_hop_count != -1){
				return h_hop_count;
			}
			else{
				get_h_hop_neighborhood(h);
				return h_hop_count; 
			}
		}
		
		set<User*> get_h_hop_neighborhood(int h){
			if(h_hop_searched){
				return h_hop;
			}
			set<User*> current;
			h_hop.insert(this);
			current.insert(this);
			for(int i=0; i<h; i++){
				if(current.empty()){
					break;
				}
				set<User*> next;
				for(set<User*>::iterator it = current.begin(); it != current.end(); it++){
					for(map<User*, FriendEdge*>::iterator friend_it = (*it)->friends.begin(); friend_it != (*it)->friends.end(); friend_it++) {
						if(h_hop.find(friend_it->first) == h_hop.end()){
							next.insert(friend_it->first);
							h_hop.insert(friend_it->first);
						}
					}				 
				}
				current = next;
				next.clear();
			}
			h_hop_count = h_hop.size();
			h_hop_searched = true;
			current.clear();
			return h_hop;
		}
		
		int get_in_h_hop_count(const set<User*> &U, int h){
			if(!h_hop_searched){
				get_h_hop_neighborhood(h);
			}
			list<User*> ret;
			set_intersection(h_hop.begin(), h_hop.end(), U.begin(), U.end(), inserter(ret, ret.begin()));
//			cout << id << ": " << ret.size()-1 << endl;;
			int ret_size = ret.size()-1;
			ret.clear();
			return ret_size; 
		}
		
		void get_users_out_of_h_hop(const set<User*> &U, int h, set<User*> &ret){
			if(!h_hop_searched){
				get_h_hop_neighborhood(h);
			}
			set_difference(U.begin(), U.end(), h_hop.begin(), h_hop.end(), inserter(ret, ret.begin()));
		}
		
		void get_users_in_h_hop(const set<User*> &U, int h, set<User*> &ret){
			if(!h_hop_searched){
				get_h_hop_neighborhood(h);
			}
			set_intersection(U.begin(), U.end(), h_hop.begin(), h_hop.end(), inserter(ret, ret.begin()));
		}
		
		bool is_in_h_hop(const set<User*> &U, int h){
			if(get_in_h_hop_count(U, h) != U.size()-1){
				return false;			
			}
			else{
				return true;
			}
		}
		
		bool is_in_h_hop(User* u, int h){
			if(h_hop_searched){
				if(h_hop.find(u) != h_hop.end()){
					return true;
				}
				else{
					false;
				}
			}
			set<User*> all;
			set<User*> current;
			all.insert(this);
			current.insert(this);
			bool ret_true = false;
			for(int i=0; i<h; i++){
				if(current.empty()){
					break;
				}
				set<User*> next;
				for(set<User*>::iterator it = current.begin(); it != current.end(); it++){
					for(map<User*, FriendEdge*>::iterator friend_it = (*it)->friends.begin(); friend_it != (*it)->friends.end(); friend_it++) {
						if(friend_it->first == u){
							ret_true = true;
							break;
						}
						if(all.find(friend_it->first) == all.end()){
							next.insert(friend_it->first);
							all.insert(friend_it->first);
						}
					}				 
				}
				current = next;
				next.clear();
			}
			all.clear();
			current.clear();
			if(ret_true){
				return true;
			}
			else{
				return false;
			}			
		}
		
		bool is_all_U_in_h_hop(const set<User*> &U, int h){
			if(!h_hop_searched){
				get_h_hop_neighborhood(h);
			}
			list<User*> ret;
			set_difference(U.begin(), U.end(), h_hop.begin(), h_hop.end(), inserter(ret, ret.begin()));
			if(ret.size() == 0){
				return true;
			}
			else{
				return false;
			}	
		}
		
		double get_sum_of_potential_weights(const set<User*> &U){
			double ret = 0.0;
			for(map<User*, PotentialEdge*>::iterator it = potential.begin(); it != potential.end(); it++){
				if(U.find(it->first) != U.end()){
					ret += it->second->weight;
				}
			}
			return ret;
		}
		
		double get_sum_of_all_potential_weights(){
			if(sum_of_all_potential_weights != -1){
				return sum_of_all_potential_weights;
			}
			else{
				sum_of_all_potential_weights = 0.0;
				for(map<User*, PotentialEdge*>::iterator it = potential.begin(); it != potential.end(); it++){
					sum_of_all_potential_weights += it->second->weight;
				}
				return sum_of_all_potential_weights;				
			}
		}
		
		double get_all_topic_engagement(const set<Item*> &O, const list<int> &topics){
			stringstream ss;
			ss << id << ": ";
			double sum = 0;
			for(map<Item*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
				if(O.find(it->first) == O.end()){
					continue;
				}
				if(relevance.find(it->first) != relevance.end()){
					double w = it->second->weight;
					for(list<int>::const_iterator t_it = topics.begin(); t_it != topics.end(); t_it++){
						if(relevance[it->first][*t_it] != 0){
							ss << " + " << w << " * " << relevance[it->first][*t_it] << " [" << *t_it << "] ";
						}						
						sum += w * relevance[it->first][*t_it];
					}					
				}
			}
			ss << " = " << sum << endl;
			if(sum != 0){
//				cout << ss.str();
			}
			return sum;				
		}
		
		double get_all_topic_engagement(const list<int> &topics){
			if(all_topic_engagement == -1){
				double sum = 0;
				for(map<Item*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
					if(relevance.find(it->first) != relevance.end()){
						double w = it->second->weight;
						for(list<int>::const_iterator t_it = topics.begin(); t_it != topics.end(); t_it++){
							sum += w * relevance[it->first][*t_it];
						}					
					}
				}
				all_topic_engagement = sum;				
			}
			return all_topic_engagement;
		}
		  		
		double get_at_least_preference(const set<Item*> &O){
			double product = 1;
			for(map<Item*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
				if(O.find(it->first) != O.end()){
					product *= (1-it->second->weight);
				}
			}				
			return 1-product;
		}
		
		double get_at_least_preference(){
			if(at_least_preference == -1){
				double product = 1;
				for(map<Item*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
					product *= (1-it->second->weight);
				}
				at_least_preference = 1-product;
			}							
			return at_least_preference;
		}
		
		void print(){
			cout << id << endl;
		}

};

class Item{
	public:
		Item(string s){
			id = s;
			type_u_i_t = 1;
		}

		string id;
		int type_u_i_t;
		map<User*, PreferenceEdge*> preference;
		map<Topic*, RelationEdge*> relation;
		
		double get_all_topic_engagement_for_U(const set<User*> &U, const list<int> &query_topics){
			double ret = 0.0;
			for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
				User* v = *it;
				if(v->preference.find(this) != v->preference.end()){					
					if(v->relevance.find(this) != v->relevance.end()){
						double w = v->preference[this]->weight;
						for(list<int>::const_iterator t_it = query_topics.begin(); t_it != query_topics.end(); t_it++){
							ret += w * v->relevance[this][*t_it];
						} 
					}					
				}
			}
			return ret;
		}
		
		double get_at_least_preference(const set<User*> &U){
			double ret = 0.0;
			for(map<User*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
				if(U.find(it->first) != U.end()){
					ret += it->second->weight;
				}			
			}
			return ret;
		}
		
		double get_at_least_preference(const set<User*> &U, const set<Item*> &O){
			double ret = 0.0;
			for(map<User*, PreferenceEdge*>::iterator it = preference.begin(); it != preference.end(); it++){
				if(U.find(it->first) != U.end()){
					User* u = it->first;
					double product = 1-(it->second->weight);
					for(map<Item*, PreferenceEdge*>::iterator user_it = u->preference.begin(); user_it != u->preference.end(); user_it++){
						if(O.find(user_it->first) != O.end()){
							product *= (1-user_it->second->weight);
						}
					}				
					ret += (1-product);
				}			
			}
			return ret;
		}
		
		void print(){
			cout << id << endl;
		}
};

class Topic{
	public:
		Topic(string s){
			id = s;
			type_u_i_t = 2;
		}
		
		string id;
		int type_u_i_t;
		map<Item*, RelationEdge*> relation;
};

class Solution{
	public:
		Solution(double d, set<User*> users, set<Item*> items){
			opt = d;
			U = users;
			O = items;			
		}
		
		Solution(double d, set<User*> users, set<Item*> items, int p){
			opt = d;
			U = users;
			O = items;
			pruned = p;			
		}
		
		double opt;
		set<User*> U;
		set<Item*> O;
		int pruned;
};

class TopItem{
	public:
		TopItem(Item* i, double v){
			item = i;
			value = v;
		}
		
		Item* item;
		double value;
};

class UserGroupGrow{
	public:
		UserGroupGrow(set<User*> &newset, int d){
			users = newset;
			offset = d;
		}
		
		UserGroupGrow(set<User*> &newset, set<User*> &p){
			users = newset;
			pool = p;
		}
		
		UserGroupGrow(set<User*> &newset, set<User*> &p, map<Item*, double> &m, double d){
			users = newset;
			pool = p;
			pte = m;
			sum_potential = d;
		}
		
		~UserGroupGrow(){
			users.clear();
			pool.clear();
			pte.clear();
		}
		
		set<User*> users;
		int offset;
		set<User*> pool;
		map<Item*, double> pte;
		double sum_potential;
		
		void print(){
			bool first = true;
			stringstream ss;
			for(set<User*>::iterator it = users.begin(); it != users.end(); it++){
				if(first){
					first = false;
				}
				else{
					ss << ", ";
				}
				ss << (*it)->id;
			}
//			ss << "\t" << offset << endl;
			first = true;
			ss << "\t";
			for(set<User*>::iterator it = pool.begin(); it != pool.end(); it++){
				if(first){
					first = false;
				}
				else{
					ss << ", ";
				}
				ss << (*it)->id;
			}
			ss << endl;
			cout << ss.str();			
		}
};


class Graph{
	public:
		Graph(){
		}
		
		list<User*> users;
		list<Item*> items;
		list<Topic*> topics;
		list<int> query_topics;
		
		User* get_random_user(){
			int index = (double)(rand() / (RAND_MAX + 1.0)) * users.size();
			list<User*>::iterator it = users.begin();
			advance(it, index);
			return *it;
		}
		
		User* get_user(string s){
			for(list<User*>::iterator it = users.begin(); it != users.end(); it++){
				User* u = *it;
				if(u->id == s){
					return u;
				}
			}
			return NULL;
		}
		
		User* get_user(string s, map<string, int> &user_mapping){
		    if(user_mapping.find(s) != user_mapping.end()){
				list<User*>::iterator it = users.begin();
				advance(it, user_mapping[s]);
				return *it;
			}
			return NULL;
		}
		
		Item* get_item(string s, map<string, int> &item_mapping){
			if(item_mapping.find(s) != item_mapping.end()){
				list<Item*>::iterator it = items.begin();
				advance(it, item_mapping[s]);
				return *it;
			}
			return NULL;
		}
				
		User* add_user(string s, map<string, int> &user_mapping){
			if(user_mapping.find(s) != user_mapping.end()){
				list<User*>::iterator it = users.begin();
				advance(it, user_mapping[s]);
				return *it;
			}
			else{
				User* u = new User(s);
				user_mapping[s] = users.size();
				users.push_back(u);
				return u;
			}
		}
		
		Item* add_item(string s, map<string, int> &item_mapping){
			if(item_mapping.find(s) != item_mapping.end()){
				list<Item*>::iterator it = items.begin();
				advance(it, item_mapping[s]);
				return *it;
			}
			else{
				Item* i = new Item(s);
				item_mapping[s] = items.size();
				items.push_back(i);
				return i;
			}
		}
		
		Topic* add_topic(string s, map<string, int> &topic_mapping){
			if(topic_mapping.find(s) != topic_mapping.end()){
				list<Topic*>::iterator it = topics.begin();
				advance(it, topic_mapping[s]);
				return *it;
			}
			else{
				Topic* t = new Topic(s);
				topic_mapping[s] = topics.size();
				topics.push_back(t);
				return t;
			}
		}
		
		int add_topic_index(string s, map<string, int> &topic_mapping){
			if(topic_mapping.find(s) != topic_mapping.end()){
				return topic_mapping[s];
			}
			else{
				int index = topic_mapping.size();
				topic_mapping[s] = index;				
				return index;
			}
		}
				
		FriendEdge* add_friend_edge(string a, string b, map<string, int> &user_mapping){
      		User* u1 = add_user(a, user_mapping);
			User* u2 = add_user(b, user_mapping);
			FriendEdge* e = new FriendEdge(u1, u2);
			u1->friends[u2] = e;
			u2->friends[u1] = e;
			return e;
		}
		
		PotentialEdge* add_potential_edge(string a, string b, double w, map<string, int> &user_mapping){
			User* u1 = add_user(a, user_mapping);
			User* u2 = add_user(b, user_mapping);
			PotentialEdge* e = new PotentialEdge(u1, u2, w);
			u1->potential[u2] = e;
			u2->potential[u1] = e;
			return e;
		}
		
		PreferenceEdge* add_preference_edge(string a, string b, double w, map<string, int> &user_mapping, map<string, int> &item_mapping){
			User* u = add_user(a, user_mapping);
			Item* i = add_item(b, item_mapping);
			PreferenceEdge* e = new PreferenceEdge(u, i, w);
			u->preference[i] = e;
			i->preference[u] = e;
			return e;
		}
		
		RelationEdge* add_relation_edge(string a, string b, map<string, int> &item_mapping, map<string, int> &topic_mapping){
			Item* i = add_item(a, item_mapping);
			Topic* t = add_topic(b, topic_mapping);
			RelationEdge* e = new RelationEdge(i, t);
			i->relation[t] = e;
			t->relation[i] = e;
			return e;
		}
		
		void add_relevance_edge(string a, string b, string c, double w, map<string, int> &user_mapping, map<string, int> &item_mapping, map<string, int> &topic_mapping){
			User* u = add_user(a, user_mapping);
			Item* i = add_item(b, item_mapping);
			int topic_index = add_topic_index(c, topic_mapping);			
			u->add_relevance(i, topic_index, w);
		}
		
		void add_query_topics(list<string> topics, map<string, int> &topic_mapping){
			for(list<string>::iterator it = topics.begin(); it != topics.end(); it++){
				int topic_index = add_topic_index(*it, topic_mapping);
//				cout << "<" << *it << ", " << topic_index << ">" << endl;
				query_topics.push_back(topic_index);	
			}
		}
		
		void sort_users(){
			users.sort([](const User* u1, const User* u2){
				if(u1->sum_of_all_potential_weights == u2->sum_of_all_potential_weights){
					return u1 > u2;
				}
				return u1->sum_of_all_potential_weights > u2->sum_of_all_potential_weights;
			});		
		}
		
		void sort_users_by_hop(){
			users.sort([](const User* u1, const User* u2){
				if(u1->h_hop_count == u2->h_hop_count){
					return u1 < u2;
				}
				return u1->h_hop_count < u2->h_hop_count;
			});		
		}
		
		
		list<set<User*>> get_feasible_user_set_at_least_n(set<User*> preset, int n, int offset, int h){
			int preset_size = preset.size();
			list<set<User*>> ret;
			list<User*>::iterator it = users.begin();
			advance(it, offset);
			for(it; it != users.end(); it++){
				set<User*> newset(preset);
				newset.insert(*it);				
				if((*it)->is_in_h_hop(newset, h)){							
					if(preset_size >= n-1){
						ret.push_back(newset);
					}
					ret.merge(get_feasible_user_set_at_least_n(newset, n, ++offset, h));
				}
			}
			return ret;		
		}
		
		Solution* get_opt_at_least_n(set<User*> preset, int n, int m, int h, int offset){
			int preset_size = preset.size();
			list<User*>::iterator it = users.begin();
			advance(it, offset);
			double max = 0.0;
			set<User*> opt_U;
			set<Item*> opt_O;			
			for(it; it != users.end(); it++){
				offset++;
				set<User*> newset(preset);
				newset.insert(*it);				
				if((*it)->is_in_h_hop(newset, h)){							
					if(preset_size >= n-1){
						set<Item*> O = opt_itemset(newset, m);
						double obj = compute_obj(newset, O);
						if(obj >= max){
							max = obj;
							opt_U = newset;
							opt_O = O;
						}					
					}
					Solution* s = get_opt_at_least_n(newset, n, m, h, offset);
					if(s->opt >= max){
						max = s->opt;
						opt_U = s->U;
						opt_O = s->O;
					}
				}
			}
			Solution* ret = new Solution(max, opt_U, opt_O);
			return ret;		
		}
		
		Solution* get_opt_at_least_n(list<UserGroupGrow*> todo, int n, int m, int h){
			double max = 0.0;
			set<User*> opt_U;
			set<Item*> opt_O;
			list<User*>::iterator it;
			
			while(todo.size() > 0){
				list<UserGroupGrow*>::iterator todo_it = todo.begin();
				UserGroupGrow* ugg = *todo_it; 
				todo.pop_front();;
				it = users.begin();				
				int new_offset = ugg->offset;
				advance(it, new_offset);				
				for(it; it != users.end(); it++){					
					new_offset++;
					User* v = *it;
					set<User*> newset(ugg->users);
					newset.insert(v);				
					if(v->is_in_h_hop(newset, h)){							
						if(newset.size() >= n){
							set<Item*> O = opt_itemset(newset, m);
							double obj = compute_obj(newset, O);
							if(obj >= max){
								max = obj;
								opt_U = newset;
								opt_O = O;
							}					
						}
						UserGroupGrow* new_ugg = new UserGroupGrow(newset, new_offset);
						todo.push_back(new_ugg);
					}					
				}
			}		
			
			Solution* ret = new Solution(max, opt_U, opt_O);
			return ret;
		}
		
		Solution* get_opt_at_least_n_new(list<UserGroupGrow*> &todo, int n, int m, int h, int thread_no){
			double max = 0.0;
			set<User*> opt_U;
			set<Item*> opt_O;
			int size = 0;
			
			while(todo.size() > 0){
				list<UserGroupGrow*>::iterator todo_it = todo.begin();			
				if((*todo_it)->users.size() > size){
					cout << "[" << thread_no << "] User group size " << (*todo_it)->users.size() << " starts" << endl;;
					size = (*todo_it)->users.size();
				}
				
				
				/*bool first = true;
				for(set<User*>::iterator u_it = (*todo_it)->users.begin(); u_it != (*todo_it)->users.end(); u_it++){
					if(first){
						first = false;
					}
					else{
						cout << ", ";
					}
					cout << (*u_it)->id;
				}
				cout << endl;
				cout << "PTE map" << endl;
				for(map<Item*, double>::iterator i_it = (*todo_it)->pte.begin(); i_it != (*todo_it)->pte.end(); i_it++){
					cout << i_it->first->id << " " << i_it->second << endl;
				}
				cout << "***" << endl;*/
				
				while((*todo_it)->users.size() + (*todo_it)->pool.size() >= n && (*todo_it)->pool.size() > 0){				
					set<User*>::iterator it = (*todo_it)->pool.begin();
					User* v = *it;
					set<User*> newset = (*todo_it)->users;
					newset.insert(v);
					
					(*todo_it)->pool.erase(v);
					set<User*> newpool;
					(v)->get_users_in_h_hop((*todo_it)->pool, h, newpool);
					
					map<Item*, double> newpte = (*todo_it)->pte;
					for(map<Item*, PreferenceEdge*>::iterator up_it = v->preference.begin(); up_it != v->preference.end(); up_it++){
						double sum = 0.0;
						if(v->relevance.find(up_it->first) != v->relevance.end()){
							double w = up_it->second->weight;
							for(list<int>::iterator utr_it = query_topics.begin(); utr_it != query_topics.end(); utr_it++){
								sum += w * v->relevance[up_it->first][*utr_it];
							}					
						}
						if(sum != 0){
							if(newpte.find(up_it->first) != newpte.end()){
								newpte[up_it->first] = newpte[up_it->first] + sum;
							}
							else{
								newpte[up_it->first] = sum;
							}
						}
					}
					
					double newpotential = (*todo_it)->sum_potential + v->get_sum_of_potential_weights((*todo_it)->users);
					
					/*cout << "\t" << v->id << endl;					
					first = true;
					for(set<User*>::iterator u_it = (*todo_it)->pool.begin(); u_it != (*todo_it)->pool.end(); u_it++){
						if(first){
							first = false;
							cout << "\t";
						}
						else{
							cout << ", ";
						}
						cout << (*u_it)->id;
					}
					cout << endl;
					first = true;
					for(set<User*>::iterator u_it = newpool.begin(); u_it != newpool.end(); u_it++){
						if(first){
							first = false;
							cout << "\t";
						}
						else{
							cout << ", ";
						}
						cout << (*u_it)->id;
					}
					cout << endl;
					cout << "\t---" << endl;				
					cout << "\tNew PTE map" << endl;
					for(map<Item*, double>::iterator i_it = newpte.begin(); i_it != newpte.end(); i_it++){
						cout << "\t" << i_it->first->id << " " << i_it->second << endl;
					}
					cout << "\t***" << endl;*/
					 
					if(newset.size() >= n){				
						set<Item*> O;
						opt_itemset(newpte, m, O);
						double topic_sum = 0.0;
						for(set<Item*>::iterator it = O.begin(); it != O.end(); it++){
							if(newpte.find(*it) != newpte.end()){
								topic_sum += newpte[*it];
							}
						}
						/*first = true;
						for(set<User*>::iterator tmp = newset.begin(); tmp != newset.end(); tmp++){
							if(first){
								first = false;
							}
							else{
								cout << ", ";
							}
							cout << (*tmp)->id;
						}
						cout << "\t";
						first = true;
						for(set<Item*>::iterator tmp = O.begin(); tmp != O.end(); tmp++){
							if(first){
								first = false;
							}
							else{
								cout << ", ";
							}
							cout << (*tmp)->id;
						}
						cout << "\t" << newpotential/newset.size() << "\t" << topic_sum/query_topics.size() << endl;*/
						double obj = newpotential/newset.size() + topic_sum/query_topics.size();
						if(obj >= max){
							max = obj;
							opt_U = newset;
							opt_O = O;
						}
						O.clear();					
					}					
					if(newset.size()+newpool.size() >= n && newpool.size() > 0){																			
						UserGroupGrow* new_ugg = new UserGroupGrow(newset, newpool, newpte, newpotential);
						todo.push_back(new_ugg);
					}
					newset.clear();
					newpool.clear();
					newpte.clear();										
				}
				todo.pop_front();
				delete *todo_it; 
			}		
			
			Solution* ret = new Solution(max, opt_U, opt_O);
			opt_U.clear();
			opt_O.clear();
			return ret;
		}
		
		double compute_obj(const set<User*> &U, const set<Item*> &O){
			double social_sum = 0.0;
			double topic_sum = 0.0;
			for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
				social_sum += (*it)->get_sum_of_potential_weights(U);
				topic_sum += (*it)->get_all_topic_engagement(O, query_topics);
			}
			return social_sum/(2*U.size()) + topic_sum/query_topics.size(); 
		}
				
		set<Item*> opt_itemset(set<User*> &U, int m){
			vector<pair<double, Item*>> top_m;
			for(list<Item*>::iterator it = items.begin(); it != items.end(); it++){
				double value = (*it)->get_all_topic_engagement_for_U(U, query_topics);
				pair<double, Item*> mypair(value, *it);
				if(top_m.size() < m){
					top_m.push_back(mypair);
					if(top_m.size() == m){
						sort_heap(top_m.begin(), top_m.end(), min_heap());
					}
				}
				else if(top_m.front() <= mypair){
						pop_heap(top_m.begin(), top_m.end(), min_heap());
						top_m.pop_back();
						top_m.push_back(mypair);
						push_heap(top_m.begin(), top_m.end(), min_heap());
						sort_heap(top_m.begin(), top_m.end(), min_heap());
				}
				else{
					// top_m.size() >=m but mypair is less than the min of top_m, so do nothing
				}
			}
			set<Item*> ret;
			for(int i=0; i<m; i++){
				ret.insert(top_m[i].second);
			}
			top_m.clear();
			return ret;
		}
		
		void opt_itemset(map<Item*, double> pte, int m, set<Item*> &ret){
			vector<pair<double, Item*>> top_m;
			for(map<Item*, double>::iterator it = pte.begin(); it != pte.end(); it++){
				Item* item = it->first;
				double value = it->second;
				pair<double, Item*> mypair(value, item);
				if(top_m.size() < m){
					top_m.push_back(mypair);
					if(top_m.size() == m){
						sort_heap(top_m.begin(), top_m.end(), min_heap());
					}
				}
				else if(top_m.front() <= mypair){
						pop_heap(top_m.begin(), top_m.end(), min_heap());
						top_m.pop_back();
						top_m.push_back(mypair);
						push_heap(top_m.begin(), top_m.end(), min_heap());
						sort_heap(top_m.begin(), top_m.end(), min_heap());
				}
				else{
					// top_m.size() >=m but mypair is less than the min of top_m, so do nothing
				}
			}
			for(int i=0; i<top_m.size(); i++){
				ret.insert(top_m[i].second);
			}
			for(list<Item*>::iterator it = items.begin(); it != items.end(); it++){
				if(ret.size() == m){
					break;
				}
				if(ret.find(*it) == ret.end()){
					ret.insert(*it);
				}
			}
			top_m.clear();
		}
		
		
		void print(){
			cout << "Users: " << endl;
      		for(list<User*>::iterator it = users.begin(); it != users.end(); it++){
				(*it)->print();				 
			}
      		cout << "Items: " << endl;
			for(list<Item*>::iterator it = items.begin(); it != items.end(); it++){
				(*it)->print();				 
			}
		}		
		
		void print_users(){
			bool first = true;
			stringstream ss;
			for(list<User*>::iterator it = users.begin(); it != users.end(); it++){
				if(first){
					first = false;
				}
				else{
					ss << ", ";
				}
				ss << (*it)->id;
			}
			cout << ss.str() << endl;			
		}
};




class Baseline{
	public:
		Baseline(string s, int i1, int i2, int i3, double t){
			name = s;
			n = i1;
			m = i2;
			h = i3;
			time = t;
		}
		
		string name;
		int n;
		int m;
		int h;
		double time;                                                                                   
		set<User*> U;
		set<Item*> O;
		double feasibility;
		double obj;
		double se;
		double pte;
		
		void update_U(string s, Graph* g, map<string, int> &user_mapping){
			size_t pos = 0;
			string token;
			while ((pos = s.find(",")) != std::string::npos) {
			    token = s.substr(0, pos);
			    User* u = g->get_user(token, user_mapping);
			    if(u != NULL){
					U.insert(u);
				}
			    s.erase(0, pos + 1);
			}
			User* u = g->get_user(s, user_mapping);
		    if(u != NULL){
				U.insert(u);
			}
		}
		
		void add_U(string token, Graph* g, map<string, int> &user_mapping){
		    User* u = g->get_user(token, user_mapping);
		    if(u != NULL){
				U.insert(u);
			}
		}
		
		void update_O(string s, Graph* g, map<string, int> &item_mapping){
			size_t pos = 0;
			string token;
			while ((pos = s.find(",")) != std::string::npos) {
			    token = s.substr(0, pos);
			    Item* i = g->get_item(token, item_mapping);
			    if(i != NULL){
					O.insert(i);
				}
			    s.erase(0, pos + 1);
			}
			Item* i = g->get_item(s, item_mapping);
		    if(i != NULL){
				O.insert(i);
			}
		}
		
		void add_O(string token, Graph* g, map<string, int> &item_mapping){
		    Item* i = g->get_item(token, item_mapping);
		    if(i != NULL){
				O.insert(i);
			}
		}
		
		void set_O(set<Item*> items){
			O = items;
		}		
		
		void set_feasibility(double f){
			feasibility = f;
		}
		
		void set_obj(double o){
			obj = o;
		}
		
		void set_se(double d){
			se = d;			
		}
		
		void set_pte(double d){
			pte = d;
		}
		
		string to_string(){
			stringstream ss;
			ss << name << "\t" << n << "\t" << m << "\t" << h << "\t" << time << "\t" << obj << "\t" << feasibility << "\t" << U.size() << "\t" << O.size() << "\t" << se << "\t" << pte << endl;
			return ss.str();
		}
				
		string to_full_string(){
			stringstream ss;			
			ss << time << "\t" << obj << "\t" << feasibility << "\t" << U.size() << "\t" << O.size() << "\t" << se << "\t" << pte << endl;
			for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
				ss << (*it)->id << endl;
			}
			for(set<Item*>::iterator it = O.begin(); it != O.end(); it++){
				ss << (*it)->id << endl;
			}
			return ss.str();
		}
		
		string to_name(){
			stringstream ss;
			ss << name << "_" << n << "_" << m << "_" << h << ".out";
			return ss.str();
		}
};

Graph* read(string input){
	Graph* g = new Graph();
	map<string, int> user_mapping;
	map<string, int> item_mapping;
	map<string, int> topic_mapping;
	
	// Read friend graph
	cout << "Strat reading friend graph" << endl;
	string filename = input+"_friend";
	fstream f1;	
	f1.open(filename.c_str(), ios::in);
	if(!f1){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f1.eof()){
			f1.getline(buffer,sizeof(buffer));
      string s = "";
			string id1 = "";			
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' '){
					id1 = s;
					s = "";
				}
				else{
				    s += buffer[i];
				}
			}
			//string id2 = s.substr(0, s.length()-1);
			string id2 = s;	
			g->add_friend_edge(id1, id2, user_mapping);
		}
	}
	
	// Read potential graph
	cout << "Strat reading potential graph" << endl;
	filename = input+"_potential";
	fstream f2;
	f2.open(filename.c_str(), ios::in);
	if(!f2){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f2.eof()){		
			f2.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_potential_edge(id1, id2, value, user_mapping);
		}
	}
	
	// Read preference graph
	cout << "Strat reading preference graph" << endl;
	filename = input+"_preference";
	fstream f3;
	f3.open(filename.c_str(), ios::in);
	if(!f3){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f3.eof()){		
			f3.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_preference_edge(id1, id2, value, user_mapping, item_mapping);
      
		}
	}
	
	// Read relevance graph
	cout << "Strat reading relevance graph" << endl;
	filename = input+"_relevance";
	fstream f4;
	f4.open(filename.c_str(), ios::in);
	if(!f4){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f4.eof()){		
			f4.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			string id3 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 2){
					id3 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_relevance_edge(id1, id2, id3, value, user_mapping, item_mapping, topic_mapping);
      
		}
	}
	
	// Read query topics
	cout << "Strat reading query topics" << endl;
	filename = input+"_query";
	fstream f5;
	f5.open(filename.c_str(), ios::in);
	if(!f5){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		f5.getline(buffer,sizeof(buffer));
    	string s = "";
    	list<string> topics;
		for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
			if(buffer[i] == ' '){
				topics.push_back(s);
				s = "";
			}
			else{
			    s += buffer[i];
			}
		}
		topics.push_back(s);
		g->add_query_topics(topics, topic_mapping);
	}
	
	
	return g;
}

Graph* read_social(string input){
	Graph* g = new Graph();
	map<string, int> user_mapping;
	
	// Read friend graph
	cout << "Strat reading friend graph" << endl;
	string filename = input+"_friend";
	fstream f1;	
	f1.open(filename.c_str(), ios::in);
	if(!f1){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f1.eof()){
			f1.getline(buffer,sizeof(buffer));
      string s = "";
			string id1 = "";			
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' '){
					id1 = s;
					s = "";
				}
				else{
				    s += buffer[i];
				}
			}
			//string id2 = s.substr(0, s.length()-1);
			string id2 = s;	
			g->add_friend_edge(id1, id2, user_mapping);
		}
	}
	
	// Read potential graph
	cout << "Strat reading potential graph" << endl;
	filename = input+"_potential";
	fstream f2;
	f2.open(filename.c_str(), ios::in);
	if(!f2){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f2.eof()){		
			f2.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_potential_edge(id1, id2, value, user_mapping);
		}
	}
	

	
	return g;
}

pair<Graph*, list<Baseline*>> read_baselines(string input){
	Graph* g = new Graph();
	map<string, int> user_mapping;
	map<string, int> item_mapping;
	map<string, int> topic_mapping;
	list<Baseline*> baselines;
	
	// Read friend graph
	cout << "Strat reading friend graph" << endl;
	string filename = input+"_friend";
	fstream f1;	
	f1.open(filename.c_str(), ios::in);
	if(!f1){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f1.eof()){
			f1.getline(buffer,sizeof(buffer));
      string s = "";
			string id1 = "";			
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' '){
					id1 = s;
					s = "";
				}
				else{
				    s += buffer[i];
				}
			}
			//string id2 = s.substr(0, s.length()-1);
			string id2 = s;	
			g->add_friend_edge(id1, id2, user_mapping);
		}
	}
	
	// Read potential graph
	cout << "Strat reading potential graph" << endl;
	filename = input+"_potential";
	fstream f2;
	f2.open(filename.c_str(), ios::in);
	if(!f2){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f2.eof()){		
			f2.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_potential_edge(id1, id2, value, user_mapping);
		}
	}
	
	// Read preference graph
	cout << "Strat reading preference graph" << endl;
	filename = input+"_preference";
	fstream f3;
	f3.open(filename.c_str(), ios::in);
	if(!f3){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f3.eof()){		
			f3.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_preference_edge(id1, id2, value, user_mapping, item_mapping);
      
		}
	}
	
	// Read relevance graph
	cout << "Strat reading relevance graph" << endl;
	filename = input+"_relevance";
	fstream f4;
	f4.open(filename.c_str(), ios::in);
	if(!f4){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f4.eof()){		
			f4.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			string id3 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 2){
					id3 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			//value = 1.0;
			g->add_relevance_edge(id1, id2, id3, value, user_mapping, item_mapping, topic_mapping);
      
		}
	}
	
	// Read query topics
	cout << "Strat reading query topics" << endl;
	filename = input+"_query";
	fstream f5;
	f5.open(filename.c_str(), ios::in);
	if(!f5){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		f5.getline(buffer,sizeof(buffer));
    	string s = "";
    	list<string> topics;
		for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
			if(buffer[i] == ' '){
				topics.push_back(s);
				s = "";
			}
			else{
			    s += buffer[i];
			}
		}
		topics.push_back(s);
		g->add_query_topics(topics, topic_mapping);
	}
	
	
	// Read baselines
	cout << "Strat reading baselines" << endl;
	filename = input+"_baseline";
	fstream f6;
	f6.open(filename.c_str(), ios::in);
	if(!f6){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f6.eof()){		
			f6.getline(buffer,sizeof(buffer));
			// format: method, n, m, h, time, U size, O size
      		string s = "";
			string name = "";
			int i1 = 0;
			int i2 = 0;
			int i3 = 0;
			double time = 0.0;
			int U_size = 0;
			int O_size = 0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					name = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					i1 = std::stoi(s);
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 2){
					i2 = std::stoi(s);
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 3){
					i3 = std::stoi(s);
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 4){
					time = std::stod(s);
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 5){
					U_size = std::stoi(s);
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			O_size = std::stoi(s);
			Baseline* b = new Baseline(name, i1, i2, i3, time);
			for(int i=0; i<U_size; i++){
				f6.getline(buffer,sizeof(buffer));
				// format: a user per line
				stringstream token;
				for(int c=0; buffer[c] != '\0' && buffer[c] != '\n' && buffer[c] != '\r'; c++){			
					token << buffer[c];
				}
				b->add_U(token.str(), g, user_mapping);
			}
			
			for(int i=0; i<O_size; i++){
				f6.getline(buffer,sizeof(buffer));
				// format: an item per line
				string token = "";
				for(int c=0; buffer[c] != '\0' && buffer[c] != '\n' && buffer[c] != '\r'; c++){
					token += buffer[c];
				}
				b->add_O(token, g, item_mapping);
			}
			
			baselines.push_back(b);
            cout << baselines.size() << " baselines read" << endl;
		}
	}
	
	
	return pair<Graph*, list<Baseline*>>(g, baselines);
}

Graph* read_relation(string input){
	Graph* g = new Graph();
	map<string, int> user_mapping;
	map<string, int> item_mapping;
	map<string, int> topic_mapping;
	
	// Read friend graph
	cout << "Strat reading friend graph" << endl;
	string filename = input+"_friend";
	fstream f1;	
	f1.open(filename.c_str(), ios::in);
	if(!f1){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];		
		while(!f1.eof()){
			f1.getline(buffer,sizeof(buffer));
      string s = "";
			string id1 = "";			
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' '){
					id1 = s;
					s = "";
				}
				else{
				    s += buffer[i];
				}
			}
			//string id2 = s.substr(0, s.length()-1);
			string id2 = s;	
			g->add_friend_edge(id1, id2, user_mapping);
		}
	}
	
	// Read preference graph
	cout << "Strat reading preference graph" << endl;
	filename = input+"_preference";
	fstream f3;
	f3.open(filename.c_str(), ios::in);
	if(!f3){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f3.eof()){		
			f3.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else if(buffer[i] == ' ' && index == 1){
					id2 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			value = std::stod(s);
			g->add_preference_edge(id1, id2, value, user_mapping, item_mapping);
      
		}
	}
	
	// Read relation graph
	cout << "Strat reading relation graph" << endl;
	filename = input+"_relation";
	fstream f4;
	f4.open(filename.c_str(), ios::in);
	if(!f4){
		cout << "Cannot open " << filename << endl;
	}
	else{
		char buffer[200];
		while(!f4.eof()){		
			f4.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != '\r'; i++){
				if(buffer[i] == ' ' && index == 0){
					id1 = s;
					s = "";
					index++;
				}
				else{
				    s += buffer[i];
				}
			}
			g->add_relation_edge(id1, s, item_mapping, topic_mapping);      
		}
	}		
	
	return g;
}

double compute_social_obj(const set<User*> &U){
	double sum = 0.0;
	for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
		sum += (*it)->get_sum_of_potential_weights(U);
	}
	return sum/(2*U.size());
}

double compute_topic_obj(const set<User*> &U, const set<Item*> &O, const list<int> &query_topics){
	double topic_sum = 0.0;
	for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
		User* u = *it;
		topic_sum += u->get_all_topic_engagement(O, query_topics);
	}
	return topic_sum/query_topics.size(); 
}

double compute_obj(const set<User*> &U, const set<Item*> &O, const list<int> &query_topics){
	double social_sum = 0.0;
	double topic_sum = 0.0;
	for(set<User*>::iterator it = U.begin(); it != U.end(); it++){
		User* u = *it;
		social_sum += u->get_sum_of_potential_weights(U);
		topic_sum += u->get_all_topic_engagement(O, query_topics);
	}
	return social_sum/(2*U.size()) + topic_sum/query_topics.size(); 
}

double compute_obj(const set<User*> &U, const set<Item*> &O, const list<int> &query_topics, map<User*, double> &tau){
	double social_sum = 0.0;
	double topic_sum = 0.0;
	for(set<User*>::const_iterator it = U.begin(); it != U.end(); it++){
		social_sum += tau[*it];
		topic_sum += (*it)->get_all_topic_engagement(O, query_topics);
	}
	return social_sum/(2*U.size()) + topic_sum/query_topics.size(); 
}

double thread_feasibility(const list<User*> &todo, const set<User*> &U, int h){
	double sum = 0.0;
	for(list<User*>::const_iterator it = todo.begin(); it != todo.end(); it++){
		sum += (*it)->get_in_h_hop_count(U, h);	
	}
	return sum;
}

double compute_feasibility(const set<User*> &U, int h, int n_thread){
	double sum = 0.0;
	
	int counter = 0;
	vector<list<User*>> todos;
	for(int i=0; i<n_thread; i++){
		list<User*> todo;
		todos.push_back(todo);
	}
	
	if(n_thread == 1){
		for(set<User*>::const_iterator it = U.begin(); it != U.end(); it++){
			sum += (double)(*it)->get_in_h_hop_count(U, h);					
		}	
	}
	else{
		for(set<User*>::const_iterator it = U.begin(); it != U.end(); it++){
			int no = counter++ % n_thread;
			todos.at(no).push_back(*it);
		}
		list<future<double>> f;
		for(int i=0; i<todos.size(); i++){
			f.push_back(std::async(std::launch::async, thread_feasibility, todos.at(i), U, h));
		}
		for(list<future<double>>::iterator f_it = f.begin(); f_it != f.end(); f_it++){
			sum += (double)(*f_it).get();
		}
	}

	return sum/(double)(U.size()*(U.size()-1));
}


set<User*> MaxGF(set<User*> H_v, int n, map<User*, double> tau){
//	map<User*, double> tau;
	double sum = 0.0;                             
	for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
//		double value = (*it)->get_sum_of_potential_weights(H_v);
//		tau[*it] = value;
		sum += tau[*it];
	}
	double max = sum/(2*H_v.size());
	set<User*> ret = H_v;
	set<User*> to_remove;

	while(H_v.size() > n){
		double min = H_v.size();
		User* selected;
		to_remove.clear();
		for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
			double value = tau[*it];		
			if(value == 0){
				to_remove.insert(*it);
			}
			else if(value <= min){
				min = value;
				selected = *it;
			}
			else{
				// do nothing
			}
		}		
		if(to_remove.size() > 0){
			set<User*> H_v_left;
			if(H_v.size() - to_remove.size() <= n){
				set<User*>::iterator to_remove_it = to_remove.begin();
				advance(to_remove_it, H_v.size()-n);
				set_difference(H_v.begin(), H_v.end(), to_remove.begin(), to_remove_it, std::inserter(H_v_left, H_v_left.end()));				
			}
			else{
				set_difference(H_v.begin(), H_v.end(), to_remove.begin(), to_remove.end(), std::inserter(H_v_left, H_v_left.end()));				
			}
			H_v = H_v_left;
			//cout << "H_v.size() = " << H_v.size() << "; to_remove.size() = " << to_remove.size() << "; H_v_left.size() = " << H_v_left.size() << endl;
		}
		else{
			H_v.erase(selected);
			sum -= min;
			for(map<User*, PotentialEdge*>::iterator p_it = selected->potential.begin(); p_it != selected->potential.end(); p_it++){
				User* pf = p_it->first;
				if(tau.find(pf) != tau.end()){
					tau[pf] = tau[pf] - p_it->second->weight; 
				}
			}
			//cout << "H_v.size() = " << H_v.size() << "; single node removed" << endl;
		}		
		double social_obj = sum/(2*H_v.size());
		if(social_obj >= max){
			max = social_obj;
			ret = H_v;
		}
/*		if(min == 0){
			recompute_tau = false;
		}
		else{
			recompute_tau = true;
		}*/
	}
	
	return ret;
}

set<User*> MaxGF(set<User*> H_v, int n){
	map<User*, double> tau;
	for(set<User*>::iterator it = H_v.begin(); it != H_v.end(); it++){
		tau[*it] = (*it)->get_sum_of_potential_weights(H_v);
	}
	
	return MaxGF(H_v, n, tau);
}