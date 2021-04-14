#include <bits/stdc++.h>
#include "KDBTree.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1>
void __f(const char* name, Arg1 && arg1){
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char* names, Arg1 && arg1, Args &&... args){
    const char* comma = strchr(names+1,','); cerr.write(names,comma-names) << " : " << arg1 << " | "; __f(comma+1, args...);
}
#else
#define trace(...)
#endif

void createQuerySet(string fileName, vector<tuple<char, float, float, float> > &queryArray, int points, int offSet = 0) {
    cerr << "Begin query creation for LSI" << endl;
    string line;
    int i = 0;

    ifstream file(fileName);
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) //Ignore headers
        {
            if (i < offSet) {
                i++;
                continue;
            }
            char type;
            float lat, lon, info;
            istringstream buf(line);
            buf >> type >> lat >> lon >> info;
            queryArray.emplace_back(make_tuple(type, lat, lon, info));
            i++;
            if (i >= points+offSet) break;
        }
        file.close();
    }
    cerr << "Finish query creation for LSI" << endl;
}

void knnQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &knnLog) {

    float p[2];
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    int k = get<3>(q);

    //cerr << "Points: " << p[0] << " | " << p[1] << endl;

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->kNNQuery(vector<float>({p[0], p[1]}), res, k);
    knnLog["knn_total " + to_string(k)] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
    knnLog["count " + to_string(k)]++;
}

void rangeQuery(tuple<char, float, float, float> q, KDBTree *hTree, vector<float> boundary, map<string, double> &rangeLog) {

    float p[2], p2[2], rs;

    p[0] = get<2>(q) - 0.01; // Inserting longitude first
    p[1] = get<1>(q) - 0.01; // Inserting latitude second
    rs   = get<3>(q);

    p2[0] = min(boundary[2], p[0] + rs*(boundary[2]+abs(boundary[0])));
    p2[1] = min(boundary[3], p[1] + rs*(boundary[3]+abs(boundary[1])));

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->rangeQuery(p, p2, res);
    rangeLog["total " + to_string(rs)]     += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
    rangeLog["count " + to_string(rs)]++;
}

void insertQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &insertLog) {

    double p[2];
    int id;
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    id = get<3>(q);

    float pf[2];
    pf[0] = get<2>(q);
    pf[1] = get<1>(q);

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->insertQuery(vector<float>({pf[0], pf[1]}), res);
    insertLog["total"] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
}

void deleteQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &deleteLog) {

    double p[2];
    int id;
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    id = get<3>(q);

    float pf[2];
    pf[0] = get<2>(q);
    pf[1] = get<1>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    map<string, double> res;
    hTree->deleteQuery(vector<float>({pf[0], pf[1]}), res);
    deleteLog["total"] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
}

void evaluate(KDBTree *hTree, vector<tuple<char, float, float, float> > queryArray, vector<float> boundary, string logFile) {

    map<string, double> deleteLog, insertLog, rangeLog, knnLog;

    cout << "Begin Querying..." << endl;
    for (auto q: queryArray) {
        if (get<0>(q) == 'k') {
            knnQuery(q, hTree, knnLog);
            knnLog["count"]++;
            //trace(knnLog["count"]);
        } else if (get<0>(q) == 'r') {
            rangeQuery(q, hTree, boundary, rangeLog);
            rangeLog["count"]++;
            //trace(rangeLog["count"]);
        } else if (get<0>(q) == 'i') {
            insertQuery(q, hTree, insertLog);
            insertLog["count"]++;
            //trace(insertLog["count"]);
        } else if (get<0>(q) == 'd') {
            deleteQuery(q, hTree, deleteLog);
            deleteLog["count"]++;
            //trace(deleteLog["count"]);
        } else cerr << "Invalid Query!!!" << endl;
    }
    cout << "Finish Querying..." << endl;

    ofstream log;
    log.open(logFile, ios_base::app);
    if (!log.is_open()) cerr << "Unable to open log.txt";

    log << "------------------Results-------------------" << endl;

    log << "------------------Range Queries-------------------" << endl;
    for (auto it = rangeLog.cbegin(); it != rangeLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------KNN Queries-------------------" << endl;
    for (auto it = knnLog.cbegin(); it != knnLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------Insert Queries-------------------" << endl;
    for (auto it = insertLog.cbegin(); it != insertLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << "------------------Delete Queries-------------------" << endl;
    for (auto it = deleteLog.cbegin(); it != deleteLog.cend(); ++it)
        log << it->first << ": " << it->second << endl;

    log << endl << "************************************************" << endl;

    log.close();
}

// main with arguments to be called by python wrapper
int main(int argCount, char **args) {

    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int branchCap = stoi(string(args[3]));
    int leafCap = stoi(string(args[4]));
    string sign = "-" + to_string(branchCap) + "-" + to_string(leafCap);

    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/data/" + queryType;
    string dataFile = projectPath + "/data/aisCleanSample1e6.txt";
    //vector<int> branchCap = {5, 10, 15, 20, 25, 50, 100, 150, 200};
    long limit = 1e6;
    long totalPoints = 1e6;
    int offset = 0;
    vector<float> boundary = {-180.0, -90.0, 180.0, 90.0};

    cout << "---Generation--- " <<  endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open()) cout << "Unable to open log.txt";
    high_resolution_clock::time_point start = high_resolution_clock::now();
    cerr << "Defining KDBTree..." << endl;
    KDBTree kt = KDBTree(leafCap, branchCap, boundary, "Orient");
    cerr << "Loading KDBTree..." << endl;
    kt.load(dataFile, limit);
    double hTreeCreationTime = duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
    log << "KDBTree Creation Time: " << hTreeCreationTime << endl;
    log << "Branch Capacity: " << branchCap << endl;
    log << "Leaf Capacity: " << leafCap << endl;
    float ktSize = kt.size();
    log << "KDBTree size in MB: " << float(ktSize/1e6) << endl;

    vector<tuple<char, float, float, float> > queryArray;
    createQuerySet(queryFile, queryArray, totalPoints, offset);

    cout << "---Evaluation--- " <<  endl;
    evaluate(&kt, queryArray, boundary, logFile);
    return 0;
}
