#include <iostream>
#include "./src/patient.cpp"
#include "./headers/Patient.h"
#include "./headers/Dentist.h"
#include "./src/dentist.cpp"
#include <vector>

using namespace std;

// load vectors here
vector<Patient> patients = loadPatients();



int main() {
    loadDentists(dentists);
    int choiceForMainPage;
    // cout << "Welcome to Dentist Sdn. Bhd." << endl;
    // cout << "Please choose any of the options below:" << endl;
    // cout << "1. Login Patient" << endl;
    // cout << "2. Login Dentist" << endl;
    // cout << "3. Login Reception" << endl;
    // cout << "4. Register as Patient" << endl;

    while (true) {
        cout << "Welcome to Dentist Sdn. Bhd." << endl;
        cout << "Please choose any of the options below:" << endl;
        cout << "1. Login Patient" << endl;
        cout << "2. Login Dentist" << endl;
        cout << "3. Login Reception" << endl;
        cout << "4. Register as Patient" << endl;
        cout << "Enter your choice: ";
        
        cin >> choiceForMainPage;
        cin.ignore();

        if (choiceForMainPage == 1) {
            loginPatient(patients);
        } else if (choiceForMainPage == 2) {
            loginDentist();
        } else if (choiceForMainPage == 3) {
            loginReception();
        } else if (choiceForMainPage == 4) {
            registerPatient(patients);
        } else cout << "Invalid choice, pls try again: ";
    }
    
    return 0;
}
