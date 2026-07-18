#ifndef DENTIST_H
#define DENTIST_H

#include <string>
#include <vector>

using namespace std;

struct Dentist {
    string id;
    string name;
    int age;
    char gender;
    string nric;
    string phoneNo;
    string email;
};

void registerDentist(vector<Dentist> dentists);
void loginDentist(vector<Dentist> dentists);
void viewDentists(vector<Dentist> dentists);

#endif
