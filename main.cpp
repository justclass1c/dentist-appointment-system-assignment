#include <iostream>
#include <sstream>
#include "./src/patient.cpp"
#include "./src/dentist.cpp"
#include "./src/appointment.cpp"
#include "./src/payment.cpp"
#include "./headers/Patient.h"
#include "./headers/Dentist.h"
#include "./headers/Appointment.h"
#include "./headers/Payment.h"
#include "./headers/Console.h"
#include <vector>

using namespace std;

vector<Patient> patients = loadPatients();

void printWelcomeMenu() {
    cout << "Welcome to Dentist Sdn. Bhd." << endl;
    cout << "Please choose any of the options below:" << endl;
    cout << "1. Login Patient" << endl;
    cout << "2. Login Dentist" << endl;
    cout << "3. Login Reception" << endl;
    cout << "4. Register as Patient" << endl;
    cout << "5. Login Admin" << endl;

    cout << "0. Exit" << endl;

}

int main() {
    loadDentists();
    loadAppointments();
    loadPaymentRecords();

    printWelcomeMenu();

    string line, note;
    while (true) {
        clearLine();
        cout << note << "Choice: " << flush;
        getline(cin, line);
        stayOnPromptLine();

        int choiceForMainPage = 0;
        stringstream ss(line);

        if (!(ss >> choiceForMainPage) || choiceForMainPage < 0 || choiceForMainPage > 5) {
            note = "[invalid choice, try again] ";
            continue;
        }

        clearLine();
        cout << "Choice: " << choiceForMainPage << endl;
        note.clear();

        if (choiceForMainPage == 0) {
            cout << "Goodbye." << endl;
            break;
        }

        if (choiceForMainPage == 1)      loginPatient(patients);
        else if (choiceForMainPage == 2) loginDentist();
        else if (choiceForMainPage == 3) loginReception();
        else if (choiceForMainPage == 4) registerPatient(patients);
        else                             loginAdmin();

        cout << endl;
        printWelcomeMenu();
    }

    return 0;
}
