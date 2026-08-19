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
};

vector<Patient> loadPatients();
void registerPatient(vector<Patient>& patients);
void loginPatient(vector<Patient>& patients);
void savePatients(vector<Patient> patients);
void viewPatientProfile(vector<Patient> patients, string currentUserID);

void mainMenu(vector<Patient> patients, const Session& current);

#endif
