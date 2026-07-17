#include "../headers/Patient.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace std;

void registerPatient(vector<Patient> patients) {
    char choice;

    do {
        Patient p;
        cout << "Name: ";
        cin >> p.name;
        cout << "NRIC: ";
        cin >> p.nric;
        cout << "Phone Number: ";
        cin >> p.phoneNo;//more info type yourself

        patients.push_back(p);

        cin.ignore();
    } while (choice == 'y' || choice == 'Y');

    ofstream outFile("C:/Users/user/Desktop/dentist assignment/patients.txt", ios::app); //write in .txt file
                                                                                         //
    for (const auto& p : patients) {
        outFile << p.name << "," << p.nric << "," << p.phoneNo << endl;
    }

    outFile << "Hi" << endl;
    outFile.close();

    cout << "Data Saved";
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
        // code here
    }
}

void createPatient(Patient patient, vector<Patient> patients) {

}
