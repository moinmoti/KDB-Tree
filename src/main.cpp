#include "KDBTree.h"

struct Stats {
    struct StatType {
        long count = 0;
        long io = 0;
        // long time = 0;
    };

    StatType del;
    StatType insert;
    map<int, StatType> knn;
    map<float, StatType> range;
    StatType reload;
};

void deleteQuery(tuple<char, vector<float>, float> q, KDBTree *index, Stats &stats) {
    Entry e;
    for (uint i = 0; i < e.pt.size(); i++)
        e.pt[i] = get<1>(q)[i];
    e.id = get<2>(q);
    Info info = index->deleteQuery(e);
    stats.del.io += info.cost;
    stats.del.count++;
}

void insertQuery(tuple<char, vector<float>, float> q, KDBTree *index, Stats &stats) {
    Entry e;
    for (uint i = 0; i < e.pt.size(); i++)
        e.pt[i] = get<1>(q)[i];
    e.id = get<2>(q);
    Info info = index->insertQuery(e);
    stats.insert.io += info.cost;
    stats.insert.count++;
}

void knnQuery(tuple<char, vector<float>, float> q, KDBTree *index, Stats &stats) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int k = get<2>(q);
    Info info = index->kNNQuery(p, k);
    stats.knn[k].io += info.cost;
    stats.knn[k].count++;
}

void rangeQuery(tuple<char, vector<float>, float> q, KDBTree *index, Stats &stats) {
    array<float, 4> query;
    for (uint i = 0; i < query.size(); i++)
        query[i] = get<1>(q)[i];
    float rs = get<2>(q);
    Info info = index->rangeQuery(query);
    stats.range[rs].io += info.cost;
    stats.range[rs].count++;
}

void evaluate(KDBTree *index, string queryFile, string logFile) {
    Stats stats;
    bool canQuery = false;
    auto roundit = [](float val, int d = 2) { return round(val * pow(10, d)) / pow(10, d); };

    cout << "Begin Querying " << queryFile << endl;
    string line;
    ifstream file(queryFile);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            tuple<char, vector<float>, float> q;
            vector<float> pts;
            if (type == 'l') {
                q = make_tuple(type, pts, 0);
            } else {
                line = line.substr(line.find_first_of(type) + 1);
                const char *cs = line.c_str();
                char *end;
                int params = (type == 'r') ? 4 : 2;
                for (uint d = 0; d < params; d++) {
                    pts.emplace_back(strtof(cs, &end));
                    cs = end;
                }
                float info = strtof(cs, &end);
                q = make_tuple(type, pts, info);
            }
            if (get<0>(q) == 'k') {
                if (canQuery)
                    knnQuery(q, index, stats);
            } else if (get<0>(q) == 'r') {
                if (canQuery)
                    rangeQuery(q, index, stats);
            } else if (get<0>(q) == 'i') {
                insertQuery(q, index, stats);
            } else if (get<0>(q) == 'd') {
                deleteQuery(q, index, stats);
            } else if (get<0>(q) == 'z') {
                stats = Stats();
                canQuery = true;
            } else if (get<0>(q) == 'l') {
                ofstream log;
                log.open(logFile, ios_base::app);
                if (!log.is_open())
                    cerr << "Unable to open log.txt";

                log << "------------------Results-------------------" << endl << endl;

                log << "------------------Range Queries-------------------" << endl;
                log << setw(8) << "Size" << setw(8) << "Count" << setw(8) << "I/O" << setw(8)
                    << "Time" << endl;
                for (auto &l : stats.range) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------KNN Queries-------------------" << endl;
                log << setw(8) << "k" << setw(8) << "Count" << setw(8) << "I/O" << setw(8) << "Time"
                    << endl;
                for (auto &l : stats.knn) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------Insert Queries-------------------" << endl;
                log << "Count:\t" << stats.insert.count << endl;
                log << "I/O:\t" << stats.insert.io / double(stats.insert.count) << endl;
                // log << "Time: \t" << stats.insert.time / double(stats.insert.count) << endl <<
                // endl;

                log << endl << "------------------ Reloading -------------------" << endl;
                log << "Count:\t" << stats.reload.count << endl;
                log << "I/O (overall):\t" << stats.reload.io << endl << endl;

                array<uint, 2> info;
                float indexSize = index->size(info);
                log << "KDBTree size in MB: " << float(indexSize / 1e6) << endl;
                log << "No. of directories: " << info[0] << endl;
                log << "No. of pages: " << info[1] << endl;

                log << endl << "************************************************" << endl;

                log.close();
            } else
                cerr << "Invalid Query!!!" << endl;
            // cerr << endl;
        }
        file.close();
    }
    cout << "Finish Querying..." << endl;
}

// main with arguments to be called by python wrapper
int main(int argCount, char **args) {
    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int fanout = stoi(string(args[3]));
    int pageCap = stoi(string(args[4]));
    // long limit = 1e8;
    /* string sign = "-I1e" + to_string(int(log10(insertions))) + "-" +
       to_string(fanout) + "-" + to_string(pageCap); */
    string sign = "-" + to_string(fanout);

    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/Data/AIS/Queries/" + queryType + ".txt";
    // string dataFile = projectPath + "/Data/AIS/ships1e8.txt";
    /* string queryFile = projectPath + "/Data/OSM/Queries/" + queryType + ".txt";
    string dataFile = projectPath + "/Data/OSM/data-7e7.txt"; */
    // vector<int> fanout = {5, 10, 15, 20, 25, 50, 100, 150, 200};
    array<float, 4> boundary{-180.0, -90.0, 180.0, 90.0};

    cout << "---Generation--- " << endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open())
        cout << "Unable to open log.txt";
    // high_resolution_clock::time_point start = high_resolution_clock::now();
    cout << "Defining KDBTree..." << endl;
    KDBTree index = KDBTree(pageCap, fanout, boundary, Spread);
    /* cout << "Bulkloading KDBTree..." << endl;
    index.bulkload(dataFile, limit); */
    /* double hTreeCreationTime =
        duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    log << "KDBTree Creation Time: " << hTreeCreationTime << endl; */
    log << "Directory Capacity: " << fanout << endl;
    log << "Page Capacity: " << pageCap << endl;
    /* map<string, double> stats;
    float indexSize = index.size(stats);
    log << "KDBTree size in MB: " << float(indexSize / 1e6) << endl;
    // index.snapshot();
    log << "No. of pages: " << stats["pages"] << endl;
    log << "No. of directories: " << stats["directories"] << endl; */

    cout << "---Evaluation--- " << endl;
    evaluate(&index, queryFile, logFile);
    // index.snapshot();
    return 0;
}
