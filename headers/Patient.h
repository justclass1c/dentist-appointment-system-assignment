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
Patient inputPatientDetails(string id, vector<Patient>& patients);
void registerPatient(vector<Patient>& patients);
void createPatient(Patient patient, vector<Patient>& patients);
void viewPatients(vector<Patient>& patients);
void loginPatient(vector<Patient>& patients);
void modifyPatient(vector<Patient>& patients, Patient& patient);
void savePatients(vector<Patient> patients);
void viewPatientProfile(vector<Patient>& patients, string currentUserID);
Patient* findPatientByID(vector<Patient>& patients, const string& id); // added: lookup used by Payment module
#endif
