#include <iostream>
#include "./src/patient.cpp"
#include "./headers/Patient.h"
#include <vector>

using namespace std;

// load vectors here
vector <Patient> patients;

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
    cout << "Welcome to Jason dentist Sdn. Bhd." << endl;
    cout << "Please choose the option below" << endl;
    cout << "1. login patient" << endl;
    cout << "2. login dentist" << endl;
    cout << "3. login reception" << endl;
    cout << "4. register as patient" << endl;

    while (true) {
        cin >> choiceForMainPage;
        if (choiceForMainPage == 1) {
            patientInfo(); //something like this
            break;
        }
        else if (choiceForMainPage == 2) {
            dentistInfo();
            break;
        }
        else if (choiceForMainPage == 3) {
            receptionInfo();
            break;
        }
        else if (choiceForMainPage == 4) {
            registerPatient(patients);
        }
        else cout << "Invalid choice, pls try again: ";
        break;
    }
    
    return 0;
}
