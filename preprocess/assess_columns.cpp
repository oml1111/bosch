#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<cassert>
#include<cmath>
#include<algorithm>
using namespace std;

const double WEIGHT = 1176868.0 / 6879.0;

vector<string> tokenize(string s) {
    vector<string> result;
    string cur;
    
    for(auto c : s) {
        if(c != ',') {
            cur.push_back(c);
        } else {
            result.push_back(cur);
            cur = "";
        }
    }
    result.push_back(cur);
    return result;
}

struct CategoricalColumn {
    struct Category {
        int count[2];
    };
    map<string, Category> categories;
    
    int total[2];
};

struct NumericColumn {
    struct Entry {
        double value;
        int response;
    };
    vector<Entry> entries;
    
    int total[2];
};

int end_categorical;
int total[2] = {0, 0};
map<string, CategoricalColumn> categorical_columns;
map<string, NumericColumn> numeric_columns;

pair<double, string> process_categorical(string name, CategoricalColumn& column) {
    map<string, double> values;
    double total_value = 0;
    
    for(auto cat : column.categories) {
        auto &c = cat.second;
        double cur_value = 2.0 * (c.count[1] * WEIGHT) / max(c.count[1] * WEIGHT + c.count[0], 1.0) - 1.0;
        values[cat.first] = cur_value;
        total_value += std::abs(cur_value) * (c.count[0] + c.count[1]) / (total[0] + total[1]);
    }
    {
        int left[2] = {total[0] - column.total[0], total[1] - column.total[1]};
        double cur_value = 2.0 * (left[1] * WEIGHT) / max(left[1] * WEIGHT + left[0], 1.0) - 1.0;
        values[""] = cur_value;
        total_value += std::abs(cur_value) * (left[0] + left[1]) / (total[0] + total[1]);
    }
    
    string output = name + "," + to_string(total_value);
    for(auto c : values) {
        output += "," + c.first + ":" + to_string(c.second);
    }
    return {total_value, output};
}

pair<double, string> process_numeric(string name, NumericColumn& column) {
    map<string, double> values;
    
    sort(column.entries.begin(), column.entries.end(), [](NumericColumn::Entry l, NumericColumn::Entry r) {
        return l.value < r.value;
    });
    
    int count[2] = {0, 0};
    double mean[2] = {0, 0};
    double relations = 0;
    for(auto c : column.entries) {
        count[c.response]++;
        mean[c.response] += c.value / column.total[c.response];
        if(c.response == 1) {
            relations += count[0];
        }
    }
    if(relations != 0) {
        relations /= 1.0 * column.total[0] * column.total[1];
    }
    
    double total_value = abs(relations * 2.0 - 1.0) * (column.total[0] + column.total[1]) / (total[0] + total[1]);
    {
        int left[2] = {total[0] - column.total[0], total[1] - column.total[1]};
        double left_frac = (left[1] * WEIGHT) / max(left[1] * WEIGHT + left[0], 1.0);
        
        values[""] = mean[1] * left_frac + mean[0] * (1 - left_frac);
        total_value += std::abs(2.0 * left_frac - 1.0) * (left[0] + left[1]) / (total[0] + total[1]);
    }
    
    string output = name + "," + to_string(total_value);
    for(auto c : values) {
        output += "," + c.first + ":" + to_string(c.second);
    }
    return {total_value, output};
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
    ifstream in1(argv[1]);
    ifstream in2(argv[2]);
    ifstream in3(argv[3]);
    
    vector<string> headers = tokenize_line(in1, in2, in3);
    for(int i=1; i < end_categorical; i++) {
        categorical_columns[headers[i]] = {{}, 0, 0};
    }
    for(int i=end_categorical; i < (int)headers.size()-1; i++) {
        numeric_columns[headers[i]] = {{}, 0, 0};
    }
    
    if(headers.back() != "Response") {
        cout << "Wrong order of arguments, must be [categorical] [date] [numeric]\n";
        cout << "Last header is: " << headers.back() << '\n';
        return 0;
    }
    
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
        
        int response = stoi(tokens.back());
        total[response]++;
        
        for (int i=1; i<end_categorical; i++) if(tokens[i] != "") {
            auto &column = categorical_columns[headers[i]];
            auto &categories = column.categories;
            if(categories.count(tokens[i]) == 0) {
                categories[tokens[i]] = {0, 0};
            }
            categories[tokens[i]].count[response]++;
            column.total[response]++;
        }
        for(int i=end_categorical; i < (int)tokens.size()-1; i++) if(tokens[i] != "") {
            auto &column = numeric_columns[headers[i]];
            auto &entries = column.entries;
            entries.push_back({stod(tokens[i]), response});
            column.total[response]++;
        }
    }
    
    vector<pair<double, string> > columns;
    for(auto c : categorical_columns) {
        columns.push_back(process_categorical(c.first, c.second));
    }
    for(auto c : numeric_columns) {
        columns.push_back(process_numeric(c.first, c.second));
    }
    sort(columns.rbegin(), columns.rend());
    
    ofstream out("columns.txt");
    for(auto c : columns) {
        out << c.second << '\n';
    }
    return 0;
}