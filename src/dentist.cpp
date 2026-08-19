#include "dentist.h"

// Hardcoded reception credentials
const string RECEPTION_USERNAME = "reception";
const string RECEPTION_PASSWORD = "pass123";

// ========================== Global Data Definitions ==========================

vector<Dentist> dentists;
vector<TimeSlot> slots;

// ========================== Helper Functions ==========================

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

// ========================== File I/O ==========================

void loadDentists(vector<Dentist>& dentists) {
    ifstream file("data/dentists.txt");
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> parts = split(line, ',');
        if (parts.size() < 5) continue;
        Dentist d;
        d.id = parts[0];
        d.name = parts[1];
        d.age = stoi(parts[2]);
        d.email = parts[3];
        d.password = parts[4];
        dentists.push_back(d);
    }
    file.close();
}

void saveDentists() {
<<<<<<< Updated upstream
    ofstream file(DENTIST_FILE);
    for (const auto& d : dentists) {
        file << d.id << "," << d.name << "," << d.age << ","
             << d.email << "," << d.password << "\n";
=======
    ofstream file("data/dentists.txt");
    for (const Dentist& d : dentists) {
        file << d.user.id << "," << d.user.name << "," << d.user.age << ","
             << d.user.email << "," << d.user.password << "\n";
>>>>>>> Stashed changes
    }
    file.close();
}

void loadSlots() {
    ifstream file("data/slots.txt");
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> parts = split(line, ',');
        if (parts.size() < 4) continue;
        TimeSlot s;
        s.dentistId = parts[0];
        s.start = parts[1];
        s.end = parts[2];
        s.available = (parts[3] == "1");
        slots.push_back(s);
    }
    file.close();
}

void saveSlots() {
    ofstream file("data/slots.txt");
    for (const auto& s : slots) {
        file << s.dentistId << "," << s.start << "," << s.end << ","
             << (s.available ? "1" : "0") << "\n";
    }
    file.close();
}

// ========================== Find Functions ==========================

Dentist* findDentistById(const string& id) {
    for (auto& d : dentists) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

Dentist* findDentistByEmail(const string email) {
    for (auto& d : dentists) {
<<<<<<< Updated upstream
        if (d.name == name) return &d;
=======
        if (d.user.email == email) return &d;
>>>>>>> Stashed changes
    }
    return nullptr;
}

vector<TimeSlot> getSlotsForDentist(const string& dentistId) {
    vector<TimeSlot> result;
    for (const auto& s : slots) {
        if (s.dentistId == dentistId) result.push_back(s);
    }
    return result;
}

void addOrUpdateSlot(const TimeSlot& slot) {
    for (auto& s : slots) {
        if (s.dentistId == slot.dentistId && s.start == slot.start && s.end == slot.end) {
            s.available = slot.available;
            return;
        }
    }
    slots.push_back(slot);
}

bool removeSlot(const string& dentistId, const string& start, const string& end) {
    for (auto it = slots.begin(); it != slots.end(); ++it) {
        if (it->dentistId == dentistId && it->start == start && it->end == end) {
            slots.erase(it);
            return true;
        }
    }
    return false;
}

// ========================== Display Functions ==========================

void displayDentistInfo(const Dentist& d) {
    cout << "ID: " << d.id << "\n";
    cout << "Name: " << d.name << "\n";
    cout << "Age: " << d.age << "\n";
    cout << "Email: " << d.email << "\n";
    cout << "Password: " << d.password << "\n";
}

void displaySlots(const vector<TimeSlot>& slotList) {
    if (slotList.empty()) {
        return;
    }
    cout << "Dentist ID\tStart\tEnd\tStatus\n";
    for (const auto& s : slotList) {
        cout << s.dentistId << "\t\t" << s.start << "\t" << s.end << "\t"
             << (s.available ? "Available" : "Locked") << "\n";
    }
}

// ========================== Admin Panel ==========================

void adminRegisterDentist() {
    Dentist d;

    if (dentists.empty()) {
        d.id = "D001";
    } else {
        string lastId = dentists.back().id;
        string numStr = lastId.substr(1); 
        int num = stoi(numStr);
        num++; 
        stringstream ss;
        ss << "D" << setw(3) << setfill('0') << num;
        d.id = ss.str();
    }
    // -----------------------

    cout << "Generated Dentist ID: " << d.id << endl;

    cout << "Enter name: ";
    cin.ignore();
<<<<<<< Updated upstream
    getline(cin, d.name);
    cout << "Enter age: ";
    cin >> d.age;
    cout << "Enter email: ";
    cin >> d.email;
    cout << "Enter password: ";
    cin >> d.password;
=======
    getline(cin, d.user.name);
    do {
        cout << "Enter age: ";
        cin >> d.user.age;
    } while (!validatePatientAge(d.user.age));
    do {
        cout << "Enter email: ";
        cin >> d.user.email;
    } while (!validateEmail(d, dentists));
    do {
        cout << "Enter password: ";
        cin >> d.user.password;
    } while (!validatePassword(d.user.password));
>>>>>>> Stashed changes

    dentists.push_back(d);
    saveDentists();
    cout << "Dentist registered successfully.\n";
}

void adminModifyDentist() {
    string id;
    cout << "Enter dentist ID to modify: ";
    cin >> id;
    //validationModifyDenitst();
    Dentist* d = findDentistById(id);
    if (d == nullptr) {
        cout << "Syntax Error: Dentist not found.\n";
        return;
    }

    cout << "Current information:\n";
    displayDentistInfo(*d);
    cout << "\nEnter new values (press Enter to keep current):\n";

    string input;
    cin.ignore();

    cout << "Name [" << d->name << "]: ";
    getline(cin, input);
    if (!input.empty()) d->name = input;

    cout << "Age [" << d->age << "]: ";
    getline(cin, input);
    if (!input.empty()) d->age = stoi(input);

    cout << "Email [" << d->email << "]: ";
    getline(cin, input);
    if (!input.empty()) d->email = input;

    cout << "Password [" << d->password << "]: ";
    getline(cin, input);
    if (!input.empty()) d->password = input;

    saveDentists();
    cout << "Dentist information updated.\n";
}

void adminPanel() {
    while (true) {
        cout << "\n--- Admin Panel ---\n";
        cout << "1. Register dentist\n";
        cout << "2. Modify dentist information\n";
        cout << "3. Back to reception menu\n";
        cout << "Choose: ";
        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                adminRegisterDentist();
                break;
            case 2:
                adminModifyDentist();
                break;
            case 3:
                return;
            default:
                cout << "Invalid choice.\n";
        }
    }
}

// ========================== Reception Menu ==========================

void receptionViewAllSchedules() {
    for (const Dentist& d : dentists) {
        vector<TimeSlot> ds = getSlotsForDentist(d.id);
        displaySlots(ds);
    }
}

void receptionMenu() {
    while (true) {
        cout << "\n--- Reception Menu ---\n";
        cout << "1. View all dentists' schedules\n";
        cout << "2. Login admin\n";
        cout << "3. Logout\n";
        cout << "Choose: ";
        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                receptionViewAllSchedules();
                break;
            case 2:
                cout << "Please enter admin name: ";
                cin >> adminNameInput;
                if (adminNameInput == adminName) {
                    cout << "Please enter admin password: ";
                    cin >> adminPasswordInput;
                    if (adminPasswordInput == adminPassword) {
                        adminPanel();
                        break;
                    }
                    else cout << "Wrong password";
                }
                else cout << "Wrong name\n";
                
            case 3:
                cout << "Logged out from reception.\n";
                return; //when return will quit programe, @jason please fix it (i dunno how to fix)
            default:
                cout << "Invalid choice.\n";
        }
    }
}

// ========================== Dentist Menu ==========================

void dentistViewSchedule(Dentist* d) {
    vector<TimeSlot> ds = getSlotsForDentist(d->id);
    displaySlots(ds);
}

void dentistAddSlot(Dentist* d) {
    TimeSlot s;
    s.dentistId = d->id;
    cout << "Enter start time (e.g., 10:00): ";
    cin >> s.start;
    cout << "Enter end time (e.g., 12:00): ";
    cin >> s.end;
    cout << "Available? (1 for yes, 0 for locked): ";
    int avail;
    cin >> avail;
    s.available = (avail == 1);
    addOrUpdateSlot(s);
    saveSlots();
    cout << "Slot added/updated.\n";
}

void dentistRemoveSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    if (removeSlot(d->id, start, end)) {
        saveSlots();
        cout << "Slot removed.\n";
    } else {
        cout << "Slot not found.\n";
    }
}

void dentistLockSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    for (auto& s : slots) {
        if (s.dentistId == d->id && s.start == start && s.end == end) {
            s.available = false;
            saveSlots();
            cout << "Slot locked (unavailable).\n";
            return;
        }
    }
    cout << "Slot not found. Do you want to create it as locked? (y/n): ";
    char resp;
    cin >> resp;
    if (tolower(resp) == 'y') {
        TimeSlot ns;
        ns.dentistId = d->id;
        ns.start = start;
        ns.end = end;
        ns.available = false;
        slots.push_back(ns);
        saveSlots();
        cout << "Slot created and locked.\n";
    } else {
        cout << "Operation cancelled.\n";
    }
}

void dentistUnlockSlot(Dentist* d) {
    string start, end;
    cout << "Enter start time: ";
    cin >> start;
    cout << "Enter end time: ";
    cin >> end;
    for (auto& s : slots) {
        if (s.dentistId == d->id && s.start == start && s.end == end) {
            s.available = true;
            saveSlots();
            cout << "Slot unlocked (available).\n";
            return;
        }
    }
    cout << "Slot not found.\n";
}

void dentistMenu(Dentist* d) {
    while (true) {
        cout << "\n--- Dentist Menu (" << d->name << ") ---\n";
        cout << "1. View my schedule\n";
        cout << "2. Manage my time slots\n";
        cout << "3. Logout\n";
        cout << "Choose: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            dentistViewSchedule(d);
        } else if (choice == 2) {
            while (true) {
                cout << "\n--- Manage Slots ---\n";
                cout << "1. Add new slot\n";
                cout << "2. Remove slot\n";
                cout << "3. Lock a slot (set unavailable)\n";
                cout << "4. Unlock a slot (set available)\n";
                cout << "5. Back\n";
                cout << "Choose: ";
                int sub;
                cin >> sub;
                switch (sub) {
                    case 1: dentistAddSlot(d); break;
                    case 2: dentistRemoveSlot(d); break;
                    case 3: dentistLockSlot(d); break;
                    case 4: dentistUnlockSlot(d); break;
                    case 5: goto back;
                    default: cout << "Invalid choice.\n";
                }
            }
            back: ;
        } else if (choice == 3) {
            cout << "Logged out.\n\n";
            return;
        } else {
            cout << "Invalid choice.\n";
        }
    }
}

// ========================== Login Functions ==========================

// void errorMsg() {
//     cout << "Dentist not found. Please try again.\n";
// }

<<<<<<< Updated upstream
    Dentist* d = findDentistByName(name);
    if (d == nullptr) {
        cout << "Dentist not found.\n";
        return;
    }
    if (d->password != pass) {
        cout << "Incorrect password.\n";
        return;
    }
    cout << "Login successful. Welcome, " << d->name << "!\n";
    dentistMenu(d);
=======
void loginDentist() {
    string pass, email, id;
    Dentist* d = findDentistById(id);

    do { 
        cout << "Enter ID: ";
        cin >> id;
        d = findDentistById(id);
        if (d == nullptr) {
            cout << "Dentist not found. Please try again.\n";
        }
    } while (d == nullptr);

    do { 
        cout << "Enter password: ";
        cin >> pass;
        if (verifyPassword (dentists, pass)) {
                cout << "Login successful. Welcome!\n";
                dentistMenu(d);
                return;
        }
        else {
            cout << "Incorrect password. Please try again.\n";
        }
    } while (!verifyPassword(dentists, pass));

    // do {
    //     cout << "Enter dentist name: ";
    //     cin.ignore();
    //     getline(cin, name);
    //     d = findDentistByName(name);
    //     if (d == nullptr) {
    //         cout << "Dentist not found.\n";
    //         return;
    // }

    //     cout << "Enter password: ";
    //     getline(cin, pass);
    //     if (d->user.password != pass) {
    //         cout << "Incorrect password.\n";
    //         return;
    //     }
    // } while (d == nullptr || d->user.password != pass);

    //Dentist* d = findDentistByName(name);
    // if (d == nullptr) {
    //     //cout << "Dentist not found.\n";
    //     return invalidDentistName();
    // }
    // if (d->user.password != pass) {
    //     cout << "Incorrect password.\n";
    //     return;
    // }

    // cout << "Login successful. Welcome, " << d->user.id << "!\n";
    // dentistMenu(d);
>>>>>>> Stashed changes
}

void loginReception() {
    string username, pass;
    cout << "Enter reception username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> pass;

    if (username == RECEPTION_USERNAME && pass == RECEPTION_PASSWORD) {
        cout << "Reception login successful.\n";
        receptionMenu();
    } else {
        cout << "Invalid credentials.\n";
    }
}

void registerPatient() {
    cout << "Patient registration not implemented (placeholder).\n";
}