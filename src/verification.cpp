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

template <typename U>
bool verifyEmail(const vector<U>& users, string input) {
    for (auto user: users) {
        if (input == user.user.email) return true;
    }
    cout << "Email format invalid, please try again.\n";
    return false;
}

template <typename U>
bool verifyPassword(const vector<U>& users, string email, string input) {
    for (auto user: users) {
        if (email == user.user.email) return input == user.user.password;
    }

    return false;
}