#include "DataStructure.h"
#include <sstream>

using namespace std;

DataStructure::DataStructure(size_t fieldCount, int initialValue)
    : fields(fieldCount, initialValue),
    locks(fieldCount)
{
}


int DataStructure::read(int index) {
    lock_guard<mutex> guard(locks.at(index));
    return fields.at(index);
}


void DataStructure::write(int index, int value) {
    lock_guard<mutex> guard(locks.at(index));
    fields.at(index) = value;
}


DataStructure::operator string() const {
   
    vector<unique_lock<mutex>> guards;
    guards.reserve(locks.size());
    for (size_t i = 0; i < locks.size(); ++i) {
        guards.emplace_back(locks[i]);
    }

    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < fields.size(); ++i) {
        ss << fields[i];
        if (i + 1 < fields.size()) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}
