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

void createQuerySet(string fileName, vector<tuple<char, float, float, float> > &queryArray) {
    cerr << "Begin query creation for LSI" << endl;
    string line;
    int i = 0;

    ifstream file(fileName);
    if (file.is_open()) {
        while (getline(file, line)) { //Ignore headers
            char type;
            float lat, lon, info;
            istringstream buf(line);
            buf >> type >> lat >> lon >> info;
            queryArray.emplace_back(make_tuple(type, lat, lon, info));
            i++;
        }
        file.close();
    }
    cerr << "Finish query creation for LSI" << endl;
}

void knnQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &knnLog) {
    array<float, 2> p;
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    int k = get<3>(q);

    //cerr << "Points: " << p[0] << " | " << p[1] << endl;

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->kNNQuery(p, res, k);
    knnLog["knn_total " + to_string(k)] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
    knnLog["count " + to_string(k)]++;
}

void rangeQuery(tuple<char, float, float, float> q, KDBTree *hTree, array<float, 4> boundary, map<string, double> &rangeLog) {
    array<float, 4> query;
    float rs;

    query[0] = get<2>(q) - 0.01; // Inserting longitude first
    query[1] = get<1>(q) - 0.01; // Inserting latitude second
    rs   = get<3>(q);

    query[2] = min(boundary[2], query[0] + rs*(boundary[2]+abs(boundary[0])));
    query[3] = min(boundary[3], query[1] + rs*(boundary[3]+abs(boundary[1])));

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->rangeQuery(query, res);
    rangeLog["total " + to_string(rs)]     += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
    rangeLog["count " + to_string(rs)]++;
}

void insertQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &insertLog) {
    array<float, 2> p;
    int id;
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    id = get<3>(q);

    map<string, double> res;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    hTree->insertQuery(p, res);
    insertLog["total"] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
}

void deleteQuery(tuple<char, float, float, float> q, KDBTree *hTree, map<string, double> &deleteLog) {
    array<float, 2> p;
    int id;
    p[0] = get<2>(q); // Inserting longitude first
    p[1] = get<1>(q); // Inserting latitude second
    id = get<3>(q);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    map<string, double> res;
    hTree->deleteQuery(p, res);
    deleteLog["total"] += duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
}

void evaluate(KDBTree *hTree, vector<tuple<char, float, float, float> > queryArray, array<float, 4> boundary, string logFile) {
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
        cerr << endl;
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
    string queryFile = projectPath + "/data/QueryFiles/" + queryType;
    string dataFile = projectPath + "/data/ships1e8.txt";
    //vector<int> branchCap = {5, 10, 15, 20, 25, 50, 100, 150, 200};
    long limit = 1e6;
    int offset = 0;
    array<float, 4> boundary{-180.0, -90.0, 180.0, 90.0};

    cout << "---Generation--- " <<  endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open()) cout << "Unable to open log.txt";
    high_resolution_clock::time_point start = high_resolution_clock::now();
    cerr << "Defining KDBTree..." << endl;
    KDBTree kt = KDBTree(leafCap, branchCap, boundary, "Orient");
    cerr << "Bulkloading KDBTree..." << endl;
    kt.bulkload(dataFile, limit);
    double hTreeCreationTime = duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
    log << "KDBTree Creation Time: " << hTreeCreationTime << endl;
    log << "Branch Capacity: " << branchCap << endl;
    log << "Leaf Capacity: " << leafCap << endl;
    float ktSize = kt.size();
    log << "KDBTree size in MB: " << float(ktSize/1e6) << endl;
    kt.snapshot();

    vector<tuple<char, float, float, float> > queryArray;
    createQuerySet(queryFile, queryArray);

    cout << "---Evaluation--- " <<  endl;
    evaluate(&kt, queryArray, boundary, logFile);
    return 0;
}
