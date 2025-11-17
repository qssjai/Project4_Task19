#pragma once

#include <vector>
#include <string>
#include <mutex>

using namespace std;


class DataStructure {
public:
    
    DataStructure(size_t fieldCount = 10, int initialValue = 0);

   
    int read(int index);

  
    void write(int index, int value);

   
    operator string() const;

    size_t size() const { return fields.size(); }

private:
    vector<int> fields;           
    mutable vector<mutex> locks;  
};