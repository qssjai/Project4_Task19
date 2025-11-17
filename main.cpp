// Compiler: Microsoft Visual Studio 2022 (MSVC), Standard: C++20
// Makarova Sofiya K27
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>
#include <sstream>

using namespace std;

#include "DataStructure.h"


bool readCommandsFromFile(const string& filename, vector<string>& commands) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: can't open file " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        commands.push_back(line);
    }
    file.close();

    if (commands.empty()) {
        cout << "File " << filename << " is empty" << endl;
        return false;
    }

    return true;
}


void executeCommands(DataStructure& data, const vector<string>& commands) {
    for (const string& line : commands) {
        if (line.empty())
            continue;

        if (line[0] == 'r') {
            // read <index>
            string cmd;
            int index;
            stringstream ss(line);
            ss >> cmd >> index;
            data.read(index);
        }
        else if (line[0] == 'w') {
            // write <index> <value>
            string cmd;
            int index, value;
            stringstream ss(line);
            ss >> cmd >> index >> value;
            data.write(index, value);
        }
        else if (line[0] == 's') {
            // string
            string snapshot = data; 
            (void)snapshot;        
        }
    }
}


void generateVariant19File(const string& filename,
    int fieldCount,
    int operationsCount,
    unsigned int seedOffset)
{
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "Cannot open file " << filename << " for writing\n";
        return;
    }

   
    (void)fieldCount; 

    mt19937 rng(static_cast<unsigned>(time(nullptr)) + seedOffset);
    uniform_real_distribution<double> prob(0.0, 1.0);

    for (int i = 0; i < operationsCount; ++i) {
        double p = prob(rng);

        // 5% read field 0
        // 40% write field 0
        // 5% read field 1
        // 5% write field 1
        // 45% string
        if (p < 0.05) {
            out << "read 0\n";
        }
        else if (p < 0.05 + 0.40) {
            out << "write 0 1\n";
        }
        else if (p < 0.05 + 0.40 + 0.05) {
            out << "read 1\n";
        }
        else if (p < 0.05 + 0.40 + 0.05 + 0.05) {
            out << "write 1 1\n";
        }
        else {
            out << "string\n";
        }
    }
}


void generateEqualFile(const string& filename,
    int fieldCount,
    int operationsCount,
    unsigned int seedOffset)
{
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "Cannot open file " << filename << " for writing\n";
        return;
    }

    mt19937 rng(static_cast<unsigned>(time(nullptr)) + seedOffset);
    uniform_int_distribution<int> typeDist(0, 2);          // 0=read,1=write,2=string
    uniform_int_distribution<int> fieldDist(0, fieldCount - 1);

    for (int i = 0; i < operationsCount; ++i) {
        int t = typeDist(rng);
        if (t == 2) {
            out << "string\n";
        }
        else if (t == 0) {
            int index = fieldDist(rng);
            out << "read " << index << "\n";
        }
        else {
            int index = fieldDist(rng);
            out << "write " << index << " 1\n";
        }
    }
}


void generateBadFile(const string& filename,
    int fieldCount,
    int operationsCount,
    unsigned int seedOffset)
{
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "Cannot open file " << filename << " for writing\n";
        return;
    }

    mt19937 rng(static_cast<unsigned>(time(nullptr)) + seedOffset);
    uniform_real_distribution<double> prob(0.0, 1.0);

    (void)fieldCount; 

    for (int i = 0; i < operationsCount; ++i) {
        double p = prob(rng);
        if (p < 0.80) {
            out << "string\n";
        }
        else if (p < 0.95) {
            out << "write 0 1\n";
        }
        else {
            out << "read 0\n";
        }
    }
}

void runTest(const string& baseName, int numThreads, int fieldCount) {
    vector<vector<string>> allCommands(numThreads);


    for (int t = 0; t < numThreads; ++t) {
        string filename = baseName + "_t" + to_string(t + 1) + ".txt";
        if (!readCommandsFromFile(filename, allCommands[t])) {
            
            cout << "Skip test for profile " << baseName
                << " with " << numThreads << " threads due to file error.\n";
            return;
        }
    }

   
    DataStructure data(fieldCount, 0);

  
    auto start = chrono::high_resolution_clock::now();

   
    vector<thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back(executeCommands,
            ref(data),
            cref(allCommands[t]));
    }

    for (auto& th : threads) {
        th.join();
    }

    // 5) Фіксуємо час
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = finish - start;

    cout << "Profile: " << baseName
        << " | Threads: " << numThreads
        << " | Time: " << duration.count() << " ms" << endl;
}


int main() {
    
    const int fieldCount = 2;          
    const int operationsCount = 200000; 
    const int maxThreads = 3;

   
    string goodBase = "GoodExpCond";   // a)
    string equalBase = "EqualExpCond";  // b)
    string badBase = "BadExpCond";    // c)

    cout << "Generating command files..." << endl;

    
    for (int t = 1; t <= maxThreads; ++t) {
        
        unsigned int seedBase = static_cast<unsigned int>(t * 100);

        string goodFileName = goodBase + "_t" + to_string(t) + ".txt";
        string equalFileName = equalBase + "_t" + to_string(t) + ".txt";
        string badFileName = badBase + "_t" + to_string(t) + ".txt";

        generateVariant19File(goodFileName, fieldCount, operationsCount, seedBase + 1);
        generateEqualFile(equalFileName, fieldCount, operationsCount, seedBase + 2);
        generateBadFile(badFileName, fieldCount, operationsCount, seedBase + 3);
    }

    cout << "Generation done.\n\nStart tests...\n\n";

    vector<string> bases = { goodBase, equalBase, badBase };
    vector<int> threadCounts = { 1, 2, 3 };

    for (const string& base : bases) {
        for (int threads : threadCounts) {
            runTest(base, threads, fieldCount);
        }
        cout << "--------------------------------------------------\n";
    }

    return 0;
}