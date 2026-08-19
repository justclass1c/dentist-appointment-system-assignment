#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

struct User {
    string id, name, email, password, nric, phoneNo;
    int age;
    char gender;
};

enum Roles {
    PATIENT, DENTIST, RECEPTIONIST, ADMIN
};

#endif
