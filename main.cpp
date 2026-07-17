#include <iostream>
#include "./src/patient.cpp"
#include "./headers/Patient.h"
#include "./src/dentist.cpp"
#include "./headers/Dentist.h"
#include <vector>

using namespace std;

// load vectors here
vector<Patient> patients = loadPatients();
vector<Dentist> dentists;

void patientInfo() {
    // idea: can prompt input and store in that array,
    // then open file and compare with the data inside the file
    // if got the data, proceed , if not go back main menu
}

void dentistInfo() {
    cout << "nah bro";
}

void receptionInfo() {
    cout << "nah sis";
}

int main() {
    int choiceForMainPage;
    cout << "Welcome to Dentist Sdn. Bhd." << endl;
    cout << "Please choose any of the options below:" << endl;
    cout << "1. Login Patient" << endl;
    cout << "2. Login Dentist" << endl;
    cout << "3. Login Reception" << endl;
    cout << "4. Register as Patient" << endl;

    while (true) {
        cin >> choiceForMainPage;

        if (choiceForMainPage == 1) {
            loginPatient(patients);
            break;
        } else if (choiceForMainPage == 2) {
            dentistInfo();
            break;
        } else if (choiceForMainPage == 3) {
            receptionInfo();
            break;
        } else if (choiceForMainPage == 4) {
            registerPatient(patients);
        } else cout << "Invalid choice, pls try again: ";
        break;
    }
    
    return 0;
}
