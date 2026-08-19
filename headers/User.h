#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

using namespace std;

struct User {
    string id, name, email, password, nric, phoneNo;
    int age;
    char gender;
};

enum Roles {
    PATIENT, DENTIST, RECEPTIONIST, ADMIN
};

void assignCurrentUser(User u);

template <typename U>
string getUsername();


#endif
