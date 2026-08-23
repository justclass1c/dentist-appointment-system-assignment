#include <vector>
#include "../headers/User.h"

string currentUserID = "";

template <typename U>
void assignCurrentUser(const vector<U>& users, string id) {
    for (auto u : users) {
        if (id == u.user.id) {
            currentUserID = u.user.id;
            break;
        }
    }
}

template <typename U>
string getUsername(const vector<U>& users, string id) {
    for (auto u : users) {
        if (id == u.user.id) {
            return u.user.name;
        }
    }
    return "";
}