#include <vector>
#include <fstream>
#include "../headers/User.h"

string currentUserID = "";

template <typename U>
void assignCurrentUser(Roles role, const vector<U>& users, string email) {
    for (auto u : users) {
        if (email == u.user.email) {
            currentUserID = u.user.id;
            break;
        };
    }
}
