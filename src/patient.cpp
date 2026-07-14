#include "../headers/Patient.h"
#include <iostream>
#include <vector>
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
