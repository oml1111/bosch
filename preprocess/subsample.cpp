#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<cassert>
#include<cmath>
#include<algorithm>
#include<random>
#include<set>
#include<deque>
using namespace std;

const double WEIGHT = 1176868.0 / 6879.0;
const int NUM_COLUMNS = 100000;
const int PICK_NUM = 6879;
const int PICK_DIV = 1176868;

mt19937 randgen;
int end_categorical;

vector<string> tokenize(string s, char separator = ',') {
    vector<string> result;
    string cur;
    
    for(auto c : s) {
        if(c != separator) {
            cur.push_back(c);
        } else {
            result.push_back(cur);
            cur = "";
        }
    }
    result.push_back(cur);
    return result;
}

vector<string> tokenize_line(istream& in1, istream& in2, istream& in3) {
    string s1, s2, s3;
    getline(in1, s1);
    getline(in2, s2);
    getline(in3, s3);
    
    vector<string> t1, t2, t3;
    t1 = tokenize(s1);
    t2 = tokenize(s2);
    t3 = tokenize(s3);
    
    end_categorical = t1.size();
    t1.insert(t1.end(), t2.begin()+1, t2.end());
    t1.insert(t1.end(), t3.begin()+1, t3.end());
    return t1;
}


int main(int argc, char* argv[]) {
    if(argc != 4) {
        cout << "Wrong no. of arguments\n";
        return 0;
    }
    ifstream colin("columns.txt");
    ifstream in1(argv[1]);
    ifstream in2(argv[2]);
    ifstream in3(argv[3]);
    deque<map<string, string> > replacements_base;
    vector<string> headers = tokenize_line(in1, in2, in3);
    vector<map<string, string>* > replacements(headers.size(), 0);
    
    bool has_response = (headers.back() == "Response");
    
    for(int i=0; i < NUM_COLUMNS; i++) {
        string s;
        
        getline(colin, s);
        vector<string> data = tokenize(s);
        if(data.size() <= 1) {
            break;
        }
        replacements_base.push_back({});
        
        for(int j=2;j<(int)data.size();j++) {
            vector<string> cur = tokenize(data[j], ':');
            replacements_base[i][cur[0]] = cur[1];
        }
        int ind = find(headers.begin(), headers.end(), data[0]) - headers.begin();
        replacements[ind] = &replacements_base[i];
    }
    
    ofstream out("combined.csv");
    out << headers[0];
    for(int i=1;i<(int)headers.size();i++) if(replacements[i] != 0) {
        out << ',' << headers[i];
    }
    if(has_response) {
        out << ',' << headers.back();
    }
    out << '\n';
    out.precision(3);
    
    while(1) {
        static int row_number = 0;
        vector<string> tokens = tokenize_line(in1, in2, in3);
        
        if(tokens.size() <= 3) {
            break;
        }
        if(row_number % 10000 == 0) {
            if(row_number > 0) {
                cout << string((string("Processing row: ") + to_string(row_number-10000) + "\n").size(), '\b');
            }
            cout << "Processing row: " << row_number << '\n';
        }
        row_number++;
        
        if(has_response && tokens.back() == "0" && randgen() % PICK_DIV >= PICK_NUM) {
            continue;
        }
        
        out << tokens[0];
        for(int i=1;i<(int)tokens.size();i++) if(replacements[i] != 0) {
            if(replacements[i]->count(tokens[i])) {
                out << ',' << replacements[i]->at(tokens[i]);
            } else if(tokens[i][0] == 'T') {
                out << ",0";
            } else {
                out << ',' << tokens[i];
            }
        }
        if(has_response) {
            out << "," << tokens.back();
        }
        out << '\n';
    }
    return 0;
}

