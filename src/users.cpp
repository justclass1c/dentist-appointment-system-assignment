#include <vector>
#include "../headers/User.h"

string currentUserID = "";

template <typename U>
void assignCurrentUser(const vector<U>& users, string email) {
    for (auto u : users) {
        if (email == u.user.email) {
            currentUserID = u.user.id;
            break;
        }
    }
}
