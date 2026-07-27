#include "dentist.h"
#include "dentist.cpp"

int main() {
    // Load data from files
    loadDentists();
    loadSlots();

    while (true) {
        cout << "\n========================================\n";
        cout << "Dental Clinic Management System\n";
        cout << "1. Login as Patient\n";
        cout << "2. Login as Dentist\n";
        cout << "3. Login as Reception\n";
        cout << "4. Register Patient\n";
        cout << "0. Exit\n";
        cout << "Choose: ";
        int option;
        cin >> option;

        switch (option) {
            case 0:
                cout << "Exiting system.\n";
                return 0;
            case 1:
                cout << "Patient login not implemented yet.\n";
                break;
            case 2:
                loginDentist();
                break;
            case 3:
                loginReception();
                break;
            case 4:
                registerPatient();
                break;
            default:
                cout << "Invalid option. Try again.\n";
        }
    }

    return 0;
}