#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <vector>
#include "User.h"
#include "Session.h"

using namespace std;

struct Patient {
    User user;
    string allergies;
    bool hasInsurance = false; // added: needed so Payment module can auto-apply insurance discount
};

vector<Patient> loadPatients();
void registerPatient(vector<Patient>& patients);
void loginPatient(vector<Patient>& patients);
void savePatients(vector<Patient> patients);
void viewPatientProfile(vector<Patient> patients, string currentUserID);
Patient* findPatientByID(vector<Patient>& patients, const string& id); // added: lookup used by Payment module

extern vector<Patient> patients; // defined in main.cpp; lets Payment look up a patient without it being passed around
#endif
