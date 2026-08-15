#include <vector>
#include <fstream>
#include "../headers/User.h"

string currentUserID = "";

template <typename U>
void assignCurrentUser(Roles role, const vector<U>& users, string email) {
    string fileName;
    if (role == PATIENT) fileName = "../data/patients.txt";
    else if (role == DENTIST) fileName = "../data/dentist.txt";

    ifstream inFile(fileName);

    for (auto u : users) {
        if (email == u.user.email) {
            currentUserID = u.user.id;
            break;
        };
    }
}
