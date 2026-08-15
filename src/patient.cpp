#include "../headers/Patient.h"
#include "../headers/Validation.h"
#include "../src/validation.cpp"
#include "../headers/Verification.h"
#include "../src/verification.cpp"
#include "../src/users.cpp"
#include <iostream>
#include <ostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;

void mainMenu(vector<Patient> patients) {
    cout << "Welcome, user!" << endl;
    cout << "1. Schedule an appointment" << endl;
    cout << "2. View appointments" << endl;
    cout << "3. View Profile" << endl;

    int input;
    cin >> input;

    switch(input) {
        case 1:
            //todo
            break;

        case 2:
            //todo
            break;

        case 3:
            viewPatientProfile(patients, currentUserID);
            break;

        default:
            cout << "Invalid input" << endl;
    }
}

void registerPatient(vector<Patient>& patients) {
    char choice;
    string tmp;
    Patient p;

    do {
        
        cout << "Name: ";
        getline(cin, p.user.name);

        while (!validatePatientAge(p.user.age)) {
            cout << "Age: ";
            cin >> p.user.age;
        }

        do {
            cout << "Gender (M/F): ";
            cin >> p.user.gender;
        } while (!validateGender(toupper(p.user.gender)));

        do {
            cout << "NRIC: ";
            cin >> p.user.nric;
        } while (!validateNRIC(p.user.nric));

        do {
            cout << "Email: ";
            cin >> p.user.email;
            cin.ignore();
<<<<<<< HEAD
        } while (!validateEmail(p));
=======
        } while (!validateEmail(p.user.email));

        do {
            cout << "Password: ";
            cin >> p.user.password;
            cin.ignore();
        } while (!validatePassword(p.user.password));
>>>>>>> a9154dd4f288ba911628396e9ad48080eba4c2be

        do {
            cout << "Phone Number: ";
            getline(cin, p.user.phoneNo);
        } while (!validatePhoneNo(p.user.phoneNo));


        cout << "Allergies: ";
        getline(cin, p.allergies);

        patients.push_back(p);

    } while (choice == 'y' || choice == 'Y');

    cout << "Your Profile (Patient ID: P" << patients.size() + 1 <<")";
    cout << "Name: " << p.user.name << endl;
    cout << "Age: " << p.user.age << endl;
    cout << "Gender: " << p.user.gender << endl;
    cout << "NRIC: " << p.user.nric << endl;
    cout << "Email: " << p.user.email << endl;
    cout << "Phone No.: " << p.user.phoneNo << endl;
    cout << "Allergies: " << p.allergies << endl;
    cout << "Confirm registration?" << endl;

    cout << "(Y = Yes / N = No / M = Nodify): ";
    savePatients(patients);

}

void loginPatient(vector<Patient>& patients) {
    string email, password;

    cout << "Please login." << endl;

    do {
        cout << "Email: ";
        cin >> email;
    } while (!verifyEmail(patients, email));


    do {
        cout << "Password: ";
        cin >> password;
    } while (!verifyPassword(patients, password));

    assignCurrentUser(PATIENT, patients, email);
    mainMenu(patients);
}

void modifyPatient(vector<Patient>& patients) {

}

vector<Patient> loadPatients() {
    ifstream inFile("data/patients.txt");

    vector<Patient> p;

    if (!inFile.is_open()) return p;

    string id, name, nric, email, password, phoneNo, allergies;
    int age;
    char gender;

    string line;

    while(getline(inFile, line)) {
        stringstream ss(line);

        string ageStr, genderStr;
        getline(ss, id, ';');
        getline(ss, name, ';');
        getline(ss, ageStr, ';');    age = stoi(ageStr);
        getline(ss, genderStr, ';'); gender = genderStr[0];
        getline(ss, nric, ';');
        getline(ss, email, ';');
        getline(ss, password, ';');
        getline(ss, phoneNo, ';');
        getline(ss, allergies);

        Patient patient;
        patient.user.id = id;
        patient.user.name = name;
        patient.user.age = age;
        patient.user.gender = gender;
        patient.user.nric = nric;
        patient.user.email = email;
        patient.user.password = password;
        patient.user.phoneNo = phoneNo;
        patient.allergies = allergies;

        p.push_back(patient);
    }

    inFile.close();

    return p;
}

void savePatients(vector<Patient> patients) {
    filesystem::create_directories("data"); // create data folder to store all text files if folder doesn't exist

    ofstream outFile("data/patients.txt");

    for (Patient patient : patients) {
        outFile << patient.user.name << ";" << patient.user.age << ";" << patient.user.gender << ";" << patient.user.nric << ";" << patient.user.email << ";" << patient.user.password << ";" << patient.user.phoneNo << ";" << patient.allergies << endl;
    }

    outFile.close();
}

void viewPatientProfile(vector<Patient> patients, string currentUserID) {
    for (Patient patient : patients) {
        if (currentUserID == patient.user.id) {
            cout << "Your Profile:" << endl;
            cout << "Patient ID: " << patient.user.id << endl;
            cout << "Name: " << patient.user.name << endl;
            cout << "Age: " << patient.user.age << endl;
            cout << "Gender: " << (patient.user.gender == 'M' ? "Male" : "Female") << endl;
            cout << "Email: " << patient.user.email << endl;
            cout << "NRIC: " << patient.user.nric << endl;
            cout << "Contact: " << patient.user.phoneNo << endl;

            break;
        }
    }
}
