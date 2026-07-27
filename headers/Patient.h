#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <vector>

using namespace std;

struct Patient {
    string id;
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
void registerPatient(vector<Patient>& patients);
void loginPatient(vector<Patient>& patients);
void viewPatients(vector<Patient>& patients);
void createPatient(Patient patient, vector<Patient>& patients);
void savePatients(vector<Patient> patients);
#endif
