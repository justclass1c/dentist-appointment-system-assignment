#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <vector>

using namespace std;

struct Patient {
    string name;
    int age;
    char gender;
    string nric;
    string email;
    string phoneNo;
    string allergies;
};

// declare functions here, then implement them in their respective cpp files
Patient inputPatientDetails(string id);
void registerPatient(vector<Patient> patients);
void loginPatient(vector<Patient> patients);
void viewPatients(vector<Patient> patients);

#endif
