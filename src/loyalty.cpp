#include "../headers/Loyalty.h"
#include "../headers/Console.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <random>
#include <algorithm>

using namespace std;

static vector<LoyaltyEntry> loyaltyEntries;
static vector<LoyaltyWin>   loyaltyWins;

const string TIER_LABEL[3] = { "Common", "Uncommon", "Rare" };
const string ENTRY_FILE = "data/loyalty_entries.txt";
const string WIN_FILE   = "data/loyalty_wins.txt";
const char   LOYALTY_DELIM = '~';

struct PrizeDef {
    PrizeTier tier;
    PrizeType type;
    string label;
    double weight;         // relative weight for discrete_distribution
    double fixedValue;     // DISCOUNT_VOUCHER: fraction (0.15/0.30/0.50); unused for SERVICE_CREDIT
    string serviceLookup;  // SERVICE_CREDIT: name to price-check in availableServices; "" otherwise
};

// weights sum to 100 -> clean percentages (Common ~75%, Uncommon ~22%, Rare ~3%)
static const PrizeDef PRIZE_TABLE[6] = {
    { COMMON,   DISCOUNT_VOUCHER, "15% Off Voucher",           50.0, 0.15, "" },
    { COMMON,   SERVICE_CREDIT,   "Free Dental Check-up",      25.0, 0.0,  "Dental Check-up" },
    { UNCOMMON, DISCOUNT_VOUCHER, "30% Off Voucher",           15.0, 0.30, "" },
    { UNCOMMON, SERVICE_CREDIT,   "Free Scaling & Polishing",   7.0, 0.0,  "Scaling & Polishing" },
    { RARE,     DISCOUNT_VOUCHER, "50% Off Voucher",            2.5, 0.50, "" },
    { RARE,     SERVICE_CREDIT,   "Free Root Canal Treatment",  0.5, 0.0,  "Root Canal Treatment" }
};

// ---------------------------------------------------------
// Small local helpers. This project merges every .cpp into one translation
// unit (main.cpp #includes them directly), so a same-named/same-signature
// `static` helper in two files is a redefinition error, not a safe pair of
// internal-linkage duplicates - hence distinct names here rather than
// reusing appointment.cpp's trimField/splitRecord/confirmPassword etc.
// ---------------------------------------------------------
static string trimLoyaltyField(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static vector<string> splitLoyaltyRecord(const string& line, char separator) {
    vector<string> fields;
    size_t start = 0;
    while (true) {
        size_t position = line.find(separator, start);
        if (position == string::npos) {
            fields.push_back(trimLoyaltyField(line.substr(start)));
            break;
        }
        fields.push_back(trimLoyaltyField(line.substr(start, position - start)));
        start = position + 1;
    }
    return fields;
}

static string todayDateStamp() {
    time_t rawTime = time(0);
    tm* localTime = localtime(&rawTime);
    stringstream ss;
    ss << setw(2) << setfill('0') << localTime->tm_mday << "/"
       << setw(2) << setfill('0') << (localTime->tm_mon + 1) << "/"
       << (localTime->tm_year + 1900);
    return ss.str();
}

static string patientDisplayName(const string& patientID) {
    Patient* p = findPatientByID(patients, patientID);
    if (p == nullptr || p->user.name.empty()) return "[" + patientID + "]";
    return p->user.name;
}

static bool confirmStaffPassword(const Session& current) {
    string note;
    const int MAX_TRIES = 3;

    for (int attempt = 1; attempt <= MAX_TRIES; attempt++) {
        stringstream label;
        label << "  Enter your password to confirm (" << attempt << " of " << MAX_TRIES << "): ";

        string entered = askInPlace(label.str(), note);
        if (!cin) return false;

        if (entered == current.password) {
            cout << "  Password confirmed.\n";
            return true;
        }
        note = "[incorrect] ";
    }
    cout << "  [!] Too many failed attempts. No changes were made.\n";
    return false;
}

static double servicePriceByName(const string& name) {
    for (size_t i = 0; i < availableServices.size(); i++) {
        if (availableServices[i].serviceName == name) return availableServices[i].price;
    }
    return 0.0;
}

static double resolvePrizeValue(const PrizeDef& prize) {
    if (prize.type == DISCOUNT_VOUCHER) return prize.fixedValue;
    return servicePriceByName(prize.serviceLookup);
}

static string sourceToString(EntrySource s) { return s == FROM_PAYMENT ? "PAYMENT" : "APPOINTMENT"; }
static EntrySource stringToSource(const string& s) { return s == "PAYMENT" ? FROM_PAYMENT : FROM_APPOINTMENT; }

static string tierToString(PrizeTier t) {
    switch (t) {
        case COMMON:   return "COMMON";
        case UNCOMMON: return "UNCOMMON";
        default:       return "RARE";
    }
}
static PrizeTier stringToTier(const string& s) {
    if (s == "UNCOMMON") return UNCOMMON;
    if (s == "RARE") return RARE;
    return COMMON;
}

static string typeToString(PrizeType t) { return t == DISCOUNT_VOUCHER ? "DISCOUNT_VOUCHER" : "SERVICE_CREDIT"; }
static PrizeType stringToType(const string& s) { return s == "SERVICE_CREDIT" ? SERVICE_CREDIT : DISCOUNT_VOUCHER; }

// ---------------------------------------------------------
// File Processing
// Record formats:
//   loyalty_entries.txt: entryID~patientID~source~sourceID~dateEarned~used
//   loyalty_wins.txt:    winID~drawID~entryID~patientID~tier~type~prizeDescription~prizeValue~drawDate~redeemed~redeemedPaymentID
// ---------------------------------------------------------
void loadLoyaltyEntries() {
    loyaltyEntries.clear();
    ifstream inFile(ENTRY_FILE.c_str());
    if (!inFile.is_open()) return;

    string line;
    while (getline(inFile, line)) {
        if (trimLoyaltyField(line).empty()) continue;
        vector<string> f = splitLoyaltyRecord(line, LOYALTY_DELIM);
        if (f.size() < 6) continue;
        if (f[0].empty() || f[1].empty()) continue;

        LoyaltyEntry e;
        e.entryID    = f[0];
        e.patientID  = f[1];
        e.source     = stringToSource(f[2]);
        e.sourceID   = f[3];
        e.dateEarned = f[4];
        e.used       = (!f[5].empty() && f[5][0] == '1');
        loyaltyEntries.push_back(e);
    }
    inFile.close();
}

void loadLoyaltyWins() {
    loyaltyWins.clear();
    ifstream inFile(WIN_FILE.c_str());
    if (!inFile.is_open()) return;

    string line;
    while (getline(inFile, line)) {
        if (trimLoyaltyField(line).empty()) continue;
        vector<string> f = splitLoyaltyRecord(line, LOYALTY_DELIM);
        if (f.size() < 11) continue;
        if (f[0].empty() || f[3].empty()) continue;

        LoyaltyWin w;
        w.winID              = f[0];
        w.drawID              = f[1];
        w.entryID             = f[2];
        w.patientID           = f[3];
        w.tier                = stringToTier(f[4]);
        w.type                = stringToType(f[5]);
        w.prizeDescription    = f[6];
        w.prizeValue          = f[7].empty() ? 0.0 : stod(f[7]);
        w.drawDate            = f[8];
        w.redeemed            = (!f[9].empty() && f[9][0] == '1');
        w.redeemedPaymentID   = f[10];
        loyaltyWins.push_back(w);
    }
    inFile.close();
}

static void saveLoyaltyEntries() {
    filesystem::create_directories("data");
    ofstream outFile(ENTRY_FILE.c_str());
    if (!outFile.is_open()) {
        cout << "  [!] Could not write to " << ENTRY_FILE << "\n";
        return;
    }
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        const LoyaltyEntry& e = loyaltyEntries[i];
        outFile << e.entryID << LOYALTY_DELIM << e.patientID << LOYALTY_DELIM << sourceToString(e.source)
                << LOYALTY_DELIM << e.sourceID << LOYALTY_DELIM << e.dateEarned << LOYALTY_DELIM
                << (e.used ? "1" : "0") << "\n";
    }
    outFile.close();
}

static void saveLoyaltyWins() {
    filesystem::create_directories("data");
    ofstream outFile(WIN_FILE.c_str());
    if (!outFile.is_open()) {
        cout << "  [!] Could not write to " << WIN_FILE << "\n";
        return;
    }
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        const LoyaltyWin& w = loyaltyWins[i];
        outFile << w.winID << LOYALTY_DELIM << w.drawID << LOYALTY_DELIM << w.entryID << LOYALTY_DELIM
                << w.patientID << LOYALTY_DELIM << tierToString(w.tier) << LOYALTY_DELIM << typeToString(w.type)
                << LOYALTY_DELIM << w.prizeDescription << LOYALTY_DELIM << fixed << setprecision(2)
                << w.prizeValue << LOYALTY_DELIM << w.drawDate << LOYALTY_DELIM << (w.redeemed ? "1" : "0")
                << LOYALTY_DELIM << w.redeemedPaymentID << "\n";
    }
    outFile.close();
}

// ---------------------------------------------------------
// ID generation (scan-for-max-suffix, same convention as generateAppointmentID/generatePaymentID)
// ---------------------------------------------------------
static string generateEntryID() {
    int highest = 0;
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        if (loyaltyEntries[i].entryID.length() <= 2) continue;
        int number = atoi(loyaltyEntries[i].entryID.substr(2).c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "LE" << setw(4) << setfill('0') << (highest + 1);
    return ss.str();
}

static string generateWinID() {
    int highest = 0;
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        if (loyaltyWins[i].winID.length() <= 2) continue;
        int number = atoi(loyaltyWins[i].winID.substr(2).c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "LW" << setw(3) << setfill('0') << (highest + 1);
    return ss.str();
}

static string generateDrawID() {
    int highest = 0;
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        const string& d = loyaltyWins[i].drawID;
        if (d.length() < 5) continue;
        int number = atoi(d.substr(4).c_str());
        if (number > highest) highest = number;
    }
    stringstream ss;
    ss << "DRAW" << setw(3) << setfill('0') << (highest + 1);
    return ss.str();
}

// ---------------------------------------------------------
// Entry-granting
// ---------------------------------------------------------
bool hasLoyaltyEntryForPayment(const string& paymentID) {
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        if (loyaltyEntries[i].source == FROM_PAYMENT && loyaltyEntries[i].sourceID == paymentID) return true;
    }
    return false;
}

bool hasLoyaltyEntryForAppointment(const string& appointmentID) {
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        if (loyaltyEntries[i].source == FROM_APPOINTMENT && loyaltyEntries[i].sourceID == appointmentID) return true;
    }
    return false;
}

static void grantEntry(const string& patientID, EntrySource source, const string& sourceID) {
    LoyaltyEntry e;
    e.entryID    = generateEntryID();
    e.patientID  = patientID;
    e.source     = source;
    e.sourceID   = sourceID;
    e.dateEarned = todayDateStamp();
    e.used       = false;
    loyaltyEntries.push_back(e);
    saveLoyaltyEntries();
}

void grantLoyaltyEntryForPayment(const string& paymentID, const string& patientID) {
    if (hasLoyaltyEntryForPayment(paymentID)) return;
    grantEntry(patientID, FROM_PAYMENT, paymentID);
}

void grantLoyaltyEntryForAppointment(const string& appointmentID, const string& patientID) {
    if (hasLoyaltyEntryForAppointment(appointmentID)) return;
    grantEntry(patientID, FROM_APPOINTMENT, appointmentID);
}

// ---------------------------------------------------------
// Staff: run the draw ceremony
// ---------------------------------------------------------
void runLoyaltyDraw(const Session& current) {
    cout << "\n" << string(58, '=') << "\n";
    cout << "  Loyalty Draw Ceremony\n";
    cout << string(58, '=') << "\n";

    vector<int> pool;
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        if (!loyaltyEntries[i].used) pool.push_back((int)i);
    }

    if (pool.empty()) {
        cout << "\n  There are no eligible loyalty tickets in the pool right now.\n";
        pauseForKey();
        return;
    }

    int maxWinners = min(MAX_DRAW_WINNERS, (int)pool.size());
    cout << "\n  " << pool.size() << " eligible ticket(s) currently in the pool.\n";

    stringstream prompt;
    prompt << "  How many winners to draw (1-" << maxWinners << ", 0 to cancel): ";
    int numWinners = readMenuChoice(prompt.str(), 0, maxWinners);
    if (numWinners == 0) {
        cout << "  Cancelled.\n";
        return;
    }

    if (!confirmStaffPassword(current)) return;

    static mt19937 rng(random_device{}());
    vector<double> weights;
    for (int i = 0; i < 6; i++) weights.push_back(PRIZE_TABLE[i].weight);
    discrete_distribution<int> prizePick(weights.begin(), weights.end());

    string drawID   = generateDrawID();
    string drawDate = todayDateStamp();

    cout << "\n  ========== DRAW " << drawID << " (" << drawDate << ") ==========\n";
    for (int w = 0; w < numWinners; w++) {
        uniform_int_distribution<size_t> ticketPick(0, pool.size() - 1);
        size_t poolPos = ticketPick(rng);
        int entryIdx = pool[poolPos];
        pool.erase(pool.begin() + poolPos); // can't win twice in the same ceremony

        LoyaltyEntry& entry = loyaltyEntries[entryIdx];
        entry.used = true;

        const PrizeDef& prize = PRIZE_TABLE[prizePick(rng)];

        LoyaltyWin win;
        win.winID             = generateWinID();
        win.drawID            = drawID;
        win.entryID           = entry.entryID;
        win.patientID         = entry.patientID;
        win.tier              = prize.tier;
        win.type              = prize.type;
        win.prizeDescription  = prize.label;
        win.prizeValue        = resolvePrizeValue(prize);
        win.drawDate          = drawDate;
        win.redeemed          = false;
        win.redeemedPaymentID = "";
        loyaltyWins.push_back(win);

        cout << "  Winner " << (w + 1) << "/" << numWinners << ":  "
             << entry.patientID << " " << patientDisplayName(entry.patientID)
             << "  ->  [" << TIER_LABEL[prize.tier] << "] " << win.prizeDescription
             << "   (Win ID " << win.winID << ", ticket " << entry.entryID << ")\n";
    }
    cout << "  " << string(58, '=') << "\n";

    saveLoyaltyEntries();
    saveLoyaltyWins();
    pauseForKey();
}

// ---------------------------------------------------------
// Staff: draw history / audit trail
// ---------------------------------------------------------
void viewLoyaltyDrawHistory() {
    if (loyaltyWins.empty()) {
        cout << "\n  No draw history yet - run a draw ceremony first.\n";
        pauseForKey();
        return;
    }

    cout << "\n  " << left << setw(9) << "WinID" << setw(10) << "DrawID"
         << setw(11) << "PatientID" << setw(10) << "Tier" << setw(28) << "Prize"
         << setw(12) << "DrawDate" << setw(10) << "Redeemed" << "RedeemedOn\n";
    cout << "  " << string(100, '-') << "\n";

    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        const LoyaltyWin& w = loyaltyWins[i];
        cout << "  " << left << setw(9) << w.winID << setw(10) << w.drawID
             << setw(11) << w.patientID << setw(10) << TIER_LABEL[w.tier]
             << setw(28) << w.prizeDescription << setw(12) << w.drawDate
             << setw(10) << (w.redeemed ? "Yes" : "No")
             << (w.redeemed ? w.redeemedPaymentID : "-") << "\n";
    }
    cout << "  " << string(100, '-') << "\n";
    cout << "  " << loyaltyWins.size() << " win record(s) shown.\n";
    pauseForKey();
}

void loyaltyMenu(const Session& current) {
    int choice;
    do {
        cout << "\n--- Loyalty Draw" << (current.role == ADMIN ? " (Admin)" : " (Reception)") << " ---\n";
        cout << "1. Run a draw ceremony\n";
        cout << "2. View draw history\n";
        cout << "0. Back\n";

        choice = readMenuChoice("Choose: ", 0, 2);

        switch (choice) {
            case 1: runLoyaltyDraw(current);  break;
            case 2: viewLoyaltyDrawHistory(); break;
            case 0: break;
        }
    } while (choice != 0);
}

// ---------------------------------------------------------
// Redemption - called from issueInvoice(), mirrors applyDiscount()
// ---------------------------------------------------------
vector<string> applyUnredeemedWin(Payment& invoice, const string& patientID) {
    vector<string> lines;

    vector<int> unredeemed;
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        if (loyaltyWins[i].patientID == patientID && !loyaltyWins[i].redeemed) {
            unredeemed.push_back((int)i);
        }
    }
    if (unredeemed.empty()) return lines;

    // Value each win would actually realize on THIS invoice - a percentage voucher
    // scales with the bill, a flat service credit is capped at what's left to pay.
    auto valueOnThisInvoice = [&](int idx) {
        const LoyaltyWin& w = loyaltyWins[idx];
        return (w.type == DISCOUNT_VOUCHER) ? (invoice.totalAmount * w.prizeValue)
                                             : min(w.prizeValue, invoice.totalAmount);
    };

    // best (highest real value on this invoice) first, so option 1 is always the recommended pick
    sort(unredeemed.begin(), unredeemed.end(), [&](int a, int b) {
        return valueOnThisInvoice(a) > valueOnThisInvoice(b);
    });

    cout << "\n  This patient has " << unredeemed.size() << " unredeemed loyalty reward(s):\n";
    cout << "  " << string(74, '-') << "\n";
    for (size_t i = 0; i < unredeemed.size(); i++) {
        const LoyaltyWin& w = loyaltyWins[unredeemed[i]];
        double wouldSave = valueOnThisInvoice(unredeemed[i]);
        cout << "  " << (i + 1) << ". [" << TIER_LABEL[w.tier] << "] " << w.prizeDescription
             << " -> saves RM " << fixed << setprecision(2) << wouldSave << " on this invoice";
        if (i == 0) cout << "  <- best value for this invoice";
        if (w.type == SERVICE_CREDIT && w.prizeValue > wouldSave) {
            cout << "\n     (only RM " << fixed << setprecision(2) << wouldSave << " of RM "
                 << fixed << setprecision(2) << w.prizeValue << " value used - RM "
                 << fixed << setprecision(2) << (w.prizeValue - wouldSave) << " would be left unused)";
        }
        cout << "\n";
    }
    cout << "  " << string(74, '-') << "\n";

    int choice = readMenuChoice("  Redeem which reward now (0 to skip - keep all pending): ", 0, (int)unredeemed.size());
    if (choice == 0) return lines;

    LoyaltyWin& w = loyaltyWins[unredeemed[choice - 1]];
    double off = (w.type == DISCOUNT_VOUCHER) ? (invoice.totalAmount * w.prizeValue) : w.prizeValue;
    invoice.totalAmount -= off;
    if (invoice.totalAmount < 0) invoice.totalAmount = 0;

    stringstream line;
    line << w.prizeDescription << " (-RM " << fixed << setprecision(2) << off << ")";
    lines.push_back(line.str());

    w.redeemed = true;
    w.redeemedPaymentID = invoice.paymentID;

    saveLoyaltyWins();
    return lines;
}

// ---------------------------------------------------------
// Patient-facing
// ---------------------------------------------------------
void checkAndShowLoyaltyNotifications(const Session& current) {
    vector<int> unredeemed;
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        if (loyaltyWins[i].patientID == current.userId && !loyaltyWins[i].redeemed) {
            unredeemed.push_back((int)i);
        }
    }
    if (unredeemed.empty()) return;

    cout << "\n  ***************************************************\n";
    cout << "  *  CONGRATULATIONS - YOU WON THE LOYALTY DRAW!     *\n";
    cout << "  ***************************************************\n";
    for (size_t i = 0; i < unredeemed.size(); i++) {
        const LoyaltyWin& w = loyaltyWins[unredeemed[i]];
        cout << "  - [" << TIER_LABEL[w.tier] << "] " << w.prizeDescription
             << "  (won " << w.drawDate << ", ID " << w.winID << ")\n";
    }
    cout << "  This will be applied automatically to your next invoice from Reception.\n";
    pauseForKey();
}

void viewMyLoyaltyTickets(const Session& current) {
    cout << "\n" << string(58, '=') << "\n";
    cout << "  My Loyalty Tickets\n";
    cout << string(58, '=') << "\n";

    int totalTickets = 0, unusedTickets = 0;
    for (size_t i = 0; i < loyaltyEntries.size(); i++) {
        if (loyaltyEntries[i].patientID != current.userId) continue;
        totalTickets++;
        if (!loyaltyEntries[i].used) unusedTickets++;
    }

    cout << "\n  Tickets earned in total   : " << totalTickets << "\n";
    cout << "  Tickets still in the pool : " << unusedTickets << "\n";

    cout << "\n  My win history\n";
    cout << "  " << string(52, '-') << "\n";
    bool any = false;
    for (size_t i = 0; i < loyaltyWins.size(); i++) {
        const LoyaltyWin& w = loyaltyWins[i];
        if (w.patientID != current.userId) continue;
        any = true;
        cout << "  [" << TIER_LABEL[w.tier] << "] " << w.prizeDescription
             << "  (won " << w.drawDate << ")  "
             << (w.redeemed ? "- redeemed" : "- NOT YET REDEEMED") << "\n";
    }
    if (!any) cout << "  You haven't won a draw yet - keep paying invoices and attending appointments!\n";
    cout << "  " << string(52, '-') << "\n";
    pauseForKey();
}
