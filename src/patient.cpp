#include "../headers/Patient.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace std;

void registerPatient(vector<Patient> patients) {
    char choice;
    Patient p;

    do {
        cout << "Name: ";
        getline(cin, p.name);
        cout << "Age: ";
        cin >> p.age;
        cout << "Gender (M/F): ";
        cin >> p.gender;
        cout << "NRIC: ";
        cin >> p.nric;
        cout << "Email: ";
        cin >> p.email;
        cout << "Phone Number: ";
        getline(cin, p.phoneNo);
        cout << "Allergies: ";
        getline(cin, p.allergies);

        patients.push_back(p);
        cin.ignore();

    } while (choice == 'y' || choice == 'Y');
                                                                                         //
    savePatients(patients);

}

void loginPatient(vector<Patient> patients) {

}

vector<Patient> loadPatients() {
    ifstream inFile("data/patients.txt");

    while (inFile.is_open()) {

    }

    vector<Patient> p;
    return p;
}

void savePatients(vector<Patient> patients) {
    filesystem::create_directories("data"); // create data folder to store all text files if folder doesn't exist

    ofstream outFile("data/patients.txt");

    for (Patient patient : patients) {
        outFile << patient.name << ";" << patient.nric << ";" << patient.phoneNo << endl;
        // code here
    }

    outFile.close();
}

