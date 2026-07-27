#include "../headers/Patient.h"
#include "../headers/Validation.h"
#include "../src/validation.cpp"
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace std;

void registerPatient(vector<Patient>& patients) {
    char choice;
    string tmp;
    Patient p;

    do {
        
        cout << "Name: ";
        getline(cin, p.name);

        while (!validatePatientAge(p.age)) {
            cout << "Age: ";
            cin >> p.age;
        }

        do {
            cout << "Gender (M/F): ";
            cin >> p.gender;
        } while (!validateGender(toupper(p.gender)));

        do {
            cout << "NRIC: ";
            cin >> p.nric;
        } while (!validateNRIC(p.nric));

        do {
            cout << "Email: ";
            cin >> p.email;
            cin.ignore();
        } while (!validateEmail(p.email));

        do {
            cout << "Phone Number: ";
            getline(cin, p.phoneNo);
        } while (!validatePhoneNo(p.phoneNo));


        cout << "Allergies: ";
        getline(cin, p.allergies);

        patients.push_back(p);

    } while (choice == 'y' || choice == 'Y');

    cout << "Your Profile (Patient ID: P" << patients.size() + 1 <<")";
    cout << "Name: " << p.name << endl;
    cout << "Age: " << p.age << endl;
    cout << "Gender: " << p.gender << endl;
    cout << "NRIC: " << p.nric << endl;
    cout << "Email: " << p.email << endl;
    cout << "Phone No.: " << p.phoneNo << endl;
    cout << "Allergies: " << p.allergies << endl;
    cout << "Confirm registration?" << endl;

    cout << "(Y = Yes / N = No / M = Nodify): ";
    savePatients(patients);

}

void loginPatient(vector<Patient>& patients) {

}

void modifyPatient(vector<Patient>& patients) {

}

vector<Patient> loadPatients() {
    ifstream inFile("data/patients.txt");

    vector<Patient> p;

    if (!inFile.is_open()) return p;

    string name, nric, email, phoneNo, allergies;
    int age;
    char gender;

    string line;

    while(getline(inFile, line)) { // check if next line exists in file
        stringstream ss(line);


        // create struct object for every line loaded
        Patient patient;
        patient.name = name;
        patient.age = age;
        patient.gender = gender;
        patient.nric = nric;
        patient.email = email;
        patient.phoneNo = phoneNo;
        patient.allergies = allergies;

        p.push_back(patient); // push the patient object to the back of the vector (append)
    }

    inFile.close();

    return p;
}

void savePatients(vector<Patient> patients) {
    filesystem::create_directories("data"); // create data folder to store all text files if folder doesn't exist

    ofstream outFile("data/patients.txt");

    for (Patient patient : patients) {
        outFile << patient.name << ";" << patient.age << ";" << patient.gender << ";" << patient.nric << ";" << patient.email << ";" << patient.phoneNo << ";" << patient.allergies << endl;
        // code here
    }

    outFile.close();
}

