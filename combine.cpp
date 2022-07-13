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
#include<future>
#include<math.h>
#include<dirent.h>
using namespace std;

string filter_mission(string filename, double threahold);

int main(int argc, char* argv[]){
    // network, threshold
	string network = argv[1];
	double th = stod(argv[2]);
			
	// load files
	string dir = ".";
    vector<string> filenames = vector<string>();
    DIR* dp;
    struct dirent* dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Cannot open " << dir << endl;
    }
    while ((dirp = readdir(dp)) != NULL) {
    	string name = dirp->d_name;
		if(name.find(network) == 0 && name.rfind(".rel") == (name.length()-4)){
			filenames.push_back(name);
		}         
    }
    closedir(dp);
    
	// combine and filter (parallel)
	stringstream ss;
	for(vector<string>::iterator it = filenames.begin(); it != filenames.end(); it++){
		ss << filter_mission(*it, th);
	}
		
	// write filter
	stringstream ss_name;
	fstream ff;
	ss_name << network << "_relevance[" << th << "]";
	ff.open(ss_name.str(), ios::out);
	ff << ss.str();
	ff.close();
}

string filter_mission(string filename, double threahold){
	cout << "Strat reading " << filename << endl;
	fstream f;	
	f.open(filename.c_str(), ios::in);
	if(!f){
		cout << "Cannot open " << filename << endl;
		return "";
	}
	else{
		char buffer[200];
		stringstream ret;
		while(!f.eof()){		
			f.getline(buffer,sizeof(buffer));
      		string s = "";
			string id1 = "";
			string id2 = "";
			string id3 = "";
			double value = 0.0;
			int index = 0;
			if(buffer[0] == '\0'){
				break;
			}
			for(int i=0; buffer[i] != '\0'; i++){
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
			if(value >= threahold){
				ret << id1 << " " << id2 << " " << id3 << " " << value << endl;
			}     
		}
		
		return ret.str();
	}
}