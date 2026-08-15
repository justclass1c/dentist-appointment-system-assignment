#include <iostream>
#include "./src/patient.cpp"
#include "./headers/Patient.h"
#include "./headers/Dentist.h"
#include <vector>

using namespace std;

// load vectors here
vector<Patient> patients = loadPatients();
vector<Dentist> dentists;

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
        cin.ignore();

        if (choiceForMainPage == 1) {
            loginPatient(patients);
            break;
        } else if (choiceForMainPage == 2) {
            // pending
            break;
        } else if (choiceForMainPage == 3) {
            // pending
            break;
        } else if (choiceForMainPage == 4) {
            registerPatient(patients);
            main();
        } else cout << "Invalid choice, pls try again: ";
        break;
    }
    
    return 0;
}
