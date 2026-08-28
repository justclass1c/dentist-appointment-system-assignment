#include <string>
#include <iostream>
#include <vector>

using namespace std;

template <typename U>
bool verifyID(const vector<U>& users, string input) {
    for (auto u: users) {
        if (input == u.user.id) return true;
    }
    
    cout << "ID is not found." << endl;
    return false;
}