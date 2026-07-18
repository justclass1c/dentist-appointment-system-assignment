#include "../headers/Dentist.h"

#include <iostream>
#include <fstream>
#include <vector>

void registerDentist(vector<Dentist> dentists) {
    char choice;

    do {
        Dentist d;
        cout << "Name: ";
        cin >> d.name;
        cout << "NRIC: ";
        cin >> d.nric;
        cout << "Phone Number: ";
        cin >> d.phoneNo;

        dentists.push_back(d);

        cin.ignore();
    } while (choice == 'y' || choice == 'Y');

    ofstream outFile("dentist.txt", ios::app); //write in .txt file
    for (const auto& d : dentists) {
        outFile << d.name << "," << d.nric << "," << d.phoneNo << endl;
    }
    outFile << "Hi" << endl;//pls delete when need to turn in
    outFile.close();
    cout << "Data Saved";
}

void loginDentistInfo() {
    string dentistName;
    double dentistIc;
    string dentistPassword;
    cout << "Please enter your name: " << endl;
    cin >> dentistName;
    cout << "Please enter your ic number: " << endl;
    cin >> dentistIc;
    cout << "Password: " << endl;
    cin >> dentistPassword;

    ifstream infile;
};

void viewDentists(vector<Dentist> dentists) {

}
