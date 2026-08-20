#include "../headers/Dentist.h"
#include "../headers/Appointment.h"
#include "../headers/Console.h"
#include "../headers/Validation.h"

const string DENTIST_FILE = "data/dentists.txt";

const string RECEPTION_USERNAME = "reception";
const string RECEPTION_PASSWORD = "pass123";

vector<Dentist> dentists;

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

vector<string> split(const string& s, char delim) {
    vector<string> tokens;
    stringstream ss(s);
    string token;
    while (getline(ss, token, delim)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

static string askNonBlank(const string& label) {
    string note, answer;
    while (true) {
        answer = trim(askInPlace(label, note));
        if (!cin) return answer;

        if (answer.empty()) {
            note = "[cannot be blank] ";
        } else if (answer.find(',') != string::npos) {
            note = "[no commas - they break the file] ";
        } else {
            acceptInPlace(label, answer);
            return answer;
        }
    }
}

void loadDentists() {
    ifstream file("data/dentists.txt");
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> parts = split(line, ',');
        if (parts.size() < 5) continue;
        if (parts[0].empty()) continue;
        if (parts[1].empty()) continue;
        Dentist d;
        d.user.id = parts[0];
        d.id = parts[0];
        d.user.name = parts[1];
        d.user.age = toIntOr(parts[2], 0);
        d.user.email = parts[3];
        d.user.password = parts[4];
        dentists.push_back(d);
    }
    file.close();
}

void saveDentists() {
    filesystem::create_directories("data");
    ofstream file(DENTIST_FILE);
    for (const Dentist& d : dentists) {
        file << d.user.id << "," << d.user.name << "," << d.user.age << ","
            << d.user.email << "," << d.user.password << "\n";
    }
    file.close();
}

Dentist* findDentistById(const string& id) {
    for (auto& d : dentists) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

Dentist* findDentistByEmail(const string& email) {
    for (auto& d : dentists) {
        if (d.user.email == email) return &d;
    }
    return nullptr;
}

void displayDentistInfo(const Dentist& d) {
    cout << "ID: " << d.id << "\n";
    cout << "Name: " << d.user.name << "\n";
    cout << "Age: " << d.user.age << "\n";
    cout << "Email: " << d.user.email << "\n";
    cout << "Password: " << string(d.user.password.length(), '*') << "\n";
}

void adminRegisterDentist() {
    Dentist d;

    int highest = 0;
    for (size_t i = 0; i < dentists.size(); i++) {
        const string& id = dentists[i].id;
        if (id.length() < 2 || toupper(id[0]) != 'D') continue;
        int number = atoi(id.substr(1).c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "D" << setw(3) << setfill('0') << (highest + 1);
    d.id = ss.str();
    d.user.id = d.id;

    cout << "Generated Dentist ID: " << d.id << endl;

    while (true) {
        d.user.name = askNonBlank("Enter name: ");
        if (!cin) break;
        if (validateName(d.user.name)) break;
        cout << "  [!] A name may only contain letters, spaces, hyphens and apostrophes.\n";
    }
    d.user.age = readMenuChoice("Enter age (18-120): ", 18, 120);

    while (true) {
        string typed = askNonBlank("Enter email (e.g. name@example.com): ");
        if (!cin) { d.user.email = typed; break; }

        d.user.email = typed;
        if (validateEmail(d, dentists)) break;
        d.user.email.clear();
        cout << "  [!] Use name@example.com, and not an email already registered.\n";
    }
    d.user.password = askNonBlank("Enter password: ");

    dentists.push_back(d);
    saveDentists();
    cout << "Dentist registered successfully.\n";
}

void adminModifyDentist() {
    if (dentists.empty()) {
        cout << "\nNo dentists are registered yet.\n";
        return;
    }

    string id, note;
    Dentist* d = NULL;

    cout << "\n  Registered dentists\n";
    cout << "  " << string(38, '-') << "\n";
    for (size_t i = 0; i < dentists.size(); i++) {
        cout << "  " << left << setw(8) << dentists[i].id << (dentists[i].user.name.empty() ? "(no name on record)" : dentists[i].user.name) << "\n";
    }
    cout << "  " << string(38, '-') << "\n";

    cout << "\nModify a dentist. (0 to cancel)\n";

    while (true) {
        id = trim(askInPlace("Dentist ID (e.g. D001): ", note));
        if (!cin || id == "0") { clearLine(); cout << "Cancelled.\n"; return; }

        d = findDentistById(id);
        if (d != NULL) break;
        note = "[no dentist with that ID] ";
    }
    acceptInPlace("Dentist ID (e.g. D001): ", id);

    cout << "\nCurrent information:\n";
    displayDentistInfo(*d);
    cout << "\nEnter new values, or leave blank to keep the current one.\n";

    string input;

    while (true) {
        input = trim(askInPlace("Name [" + d->user.name + "]: ", note));
        if (input.empty()) break;
        if (input.find(',') == string::npos) { d->user.name = input; break; }
        note = "[no commas - they break the file] ";
    }
    note.clear();
    acceptInPlace("Name: ", d->user.name);

    while (true) {
        stringstream label;
        label << "Age [" << d->user.age << "]: ";
        input = trim(askInPlace(label.str(), note));
        if (input.empty()) { acceptInPlace("Age: ", to_string(d->user.age)); break; }

        int value = 0;
        stringstream parse(input);
        if ((parse >> value) && value >= 18 && value <= 120) {
            d->user.age = value;
            acceptInPlace("Age: ", to_string(value));
            break;
        }
        note = "[18-120, or blank to keep] ";
    }
    note.clear();

    while (true) {
        input = trim(askInPlace("Email [" + d->user.email + "]: ", note));
        if (input.empty()) break;
        if (input.find(',') == string::npos) { d->user.email = input; break; }
        note = "[no commas - they break the file] ";
    }
    note.clear();
    acceptInPlace("Email: ", d->user.email);

    while (true) {
        input = trim(askInPlace("Password (blank = keep): ", note));
        if (input.empty()) break;
        if (input.find(',') == string::npos) { d->user.password = input; break; }
        note = "[no commas - they break the file] ";
    }
    note.clear();
    acceptInPlace("Password: ", string(d->user.password.length(), '*'));

    saveDentists();
    cout << "\nDentist information updated.\n";
}

void adminPanel() {
    while (true) {
        cout << "\n--- Admin Panel ---\n";
        cout << "1. Register dentist\n";
        cout << "2. Modify dentist information\n";
        cout << "3. Manage all appointments\n";
        cout << "0. Logout\n";

        int choice = readMenuChoice("Choose: ", 0, 3);

        switch (choice) {
            case 1:
                adminRegisterDentist();
                break;
            case 2:
                adminModifyDentist();
                break;
            case 3: {

                Session current;
                current.userId   = "ADM001";
                current.name     = "Administrator";
                current.password = adminPassword;
                current.role     = ADMIN;
                appointmentMenu(current);
                break;
            }
            case 0:
                cout << "Logging out.\n";
                return;
        }
    }
}

void receptionViewAllDentists() {
    if (dentists.empty()) {
        cout << "\nNo dentists are registered yet.\n" << "An admin can add one from the main menu: Login Admin > Register dentist.\n";
        return;
    }

    cout << "\n  " << left << setw(8) << "ID" << setw(24) << "Name"
        << setw(28) << "Email" << "Age\n";
    cout << "  " << string(64, '-') << "\n";
    for (size_t i = 0; i < dentists.size(); i++) {
        const Dentist& d = dentists[i];
        cout << "  " << left << setw(8) << d.id << setw(24) << (d.user.name.empty() ? "(no name on record)" : d.user.name) << setw(28) << d.user.email << d.user.age << "\n";
    }
    cout << "  " << string(64, '-') << "\n";
    cout << "  " << dentists.size() << " dentist(s).\n";
    pauseForKey();
}

void receptionMenu() {
    while (true) {
        cout << "\n--- Reception Menu ---\n";
        cout << "1. View all dentists\n";
        cout << "2. Manage appointments\n";
        cout << "0. Logout\n";

        int choice = readMenuChoice("Choose: ", 0, 2);

        switch (choice) {
            case 1:
                receptionViewAllDentists();
                break;
            case 2: {

                Session current;
                current.userId   = "R001";
                current.name     = "Reception";
                current.password = RECEPTION_PASSWORD;
                current.role     = RECEPTIONIST;
                appointmentMenu(current);
                break;
            }
            case 0:
                cout << "Logging out.\n";
                return;
        }
    }
}

void dentistMenu(Dentist* d) {
    while (true) {
        cout << "\n--- Dentist Menu (" << d->user.name << ") ---\n";
        cout << "1. My appointments\n";
        cout << "0. Logout\n";

        int choice = readMenuChoice("Choose: ", 0, 1);

        if (choice == 1) {
            Session current;
            current.userId   = d->id;
            current.name     = d->user.name;
            current.password = d->user.password;
            current.role     = DENTIST;
            appointmentMenu(current);
        } else {
            cout << "Logging out.\n";
            return;
        }
    }
}

void loginDentist() {
    string email, pass, note;

    if (dentists.empty()) {
        cout << "No dentists are registered yet.\n" << "An admin can add one from the main menu: Login Admin > Register dentist.\n";
        return;
    }

    cout << "Dentist login. (0 to go back)\n";

    Dentist* d = nullptr;
    while (true) {
        email = trim(askInPlace("Dentist email: ", note));
        if (!cin || email == "0") { clearLine(); cout << "Login cancelled.\n"; return; }

        if (email.empty()) { note = "[cannot be blank] "; continue; }

        d = findDentistByEmail(email);
        if (d != nullptr) break;
        note = "[no dentist with that email] ";
    }
    acceptInPlace("Dentist email: ", email);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass.empty()) { note = "[cannot be blank] "; continue; }
        if (pass == d->user.password) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Password: ", string(pass.length(), '*'));

    cout << "Login successful. Welcome, " << d->user.name << "!\n";
    dentistMenu(d);
}

void loginReception() {
    string username, pass, note;

    cout << "Reception login. (0 to go back)\n";

    while (true) {
        username = trim(askInPlace("Username: ", note));
        if (!cin || username == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (username == RECEPTION_USERNAME) break;
        note = "[no such user] ";
    }
    acceptInPlace("Username: ", username);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass == RECEPTION_PASSWORD) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Password: ", string(pass.length(), '*'));

    cout << "Reception login successful.\n";
    receptionMenu();
}

void loginAdmin() {
    string name, pass, note;

    cout << "Admin login. (0 to go back)\n";

    while (true) {
        name = trim(askInPlace("Admin name: ", note));
        if (!cin || name == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (name == adminName) break;
        note = "[no such admin] ";
    }
    acceptInPlace("Admin name: ", name);

    note.clear();
    while (true) {
        pass = trim(askInPlace("Admin password: ", note));
        if (!cin || pass == "0") { clearLine(); cout << "Login cancelled.\n"; return; }
        if (pass == adminPassword) break;
        note = "[incorrect password] ";
    }
    acceptInPlace("Admin password: ", string(pass.length(), '*'));

    cout << "Admin login successful.\n";
    adminPanel();
}
