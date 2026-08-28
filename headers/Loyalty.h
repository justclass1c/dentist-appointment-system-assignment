#ifndef LOYALTY_H
#define LOYALTY_H

#include <string>
#include <vector>
#include "Session.h"
#include "Payment.h" // for the Payment struct (redemption mutates one) and the ServiceItem catalog

using namespace std;

// Named Constants
const int MAX_DRAW_WINNERS = 10; // sanity cap on how many winners staff can draw in one ceremony
const int LOYALTY_EXPIRY_DAYS = 90; // an unredeemed win can no longer be applied after this many days

// How a ticket was earned - lets the win/audit trail show provenance
enum EntrySource { FROM_PAYMENT, FROM_APPOINTMENT };

// Prize catalog dimensions
enum PrizeTier { COMMON, UNCOMMON, RARE };
enum PrizeType { DISCOUNT_VOUCHER, SERVICE_CREDIT };

// One raffle ticket. Never deleted - "used" only ever flips true the moment
// this exact ticket is drawn as a winner. A patient's other tickets (won or
// not) simply stay in the pool for future draws - only the winning ticket
// itself is consumed, not the rest of that patient's entries. This is what
// makes the draw proportional: more tickets earned = better odds.
struct LoyaltyEntry {
    string entryID;      // "LE0001", "LE0002", ...
    string patientID;
    EntrySource source;  // FROM_PAYMENT or FROM_APPOINTMENT
    string sourceID;     // the paymentID or appointmentID that earned this ticket
                          // (provenance, and the de-dup guard key)
    string dateEarned;
    bool used;
};

// One drawn outcome. Created the instant a ticket wins; offered for redemption
// (reception's choice, not automatic) the next time Reception issues that
// patient any invoice (see issueInvoice()).
struct LoyaltyWin {
    string winID;              // "LW001", "LW002", ...
    string drawID;              // "DRAW001" - groups winners chosen in the same ceremony run
    string entryID;             // which specific LoyaltyEntry ticket won
    string patientID;           // denormalized, same reasoning Payment keeps patientID directly
    PrizeTier tier;
    PrizeType type;
    string prizeDescription;    // e.g. "30% Off Voucher" or "Free Root Canal Treatment"
    double prizeValue;          // DISCOUNT_VOUCHER: fraction 0-1 (0.30 = 30%);
                                 // SERVICE_CREDIT: RM amount, captured from availableServices at draw time
    string drawDate;
    string expiryDate;          // drawDate + LOYALTY_EXPIRY_DAYS, fixed the moment the win is created -
                                 // once today is past this, an unredeemed win can no longer be applied
    bool redeemed;
    string redeemedPaymentID;   // "" until redeemed, then the paymentID it was applied to
};

// File Processing
void loadLoyaltyEntries();
void loadLoyaltyWins();

// Entry-granting - called from appointment.cpp / payment.cpp at the exact
// moment an entry is earned. Both are internally guarded against double-grant.
void grantLoyaltyEntryForPayment(const string& paymentID, const string& patientID);
void grantLoyaltyEntryForAppointment(const string& appointmentID, const string& patientID);
bool hasLoyaltyEntryForPayment(const string& paymentID);
bool hasLoyaltyEntryForAppointment(const string& appointmentID);

// Staff-run ceremony + audit trail (Reception/Admin only)
void loyaltyMenu(const Session& current);
void runLoyaltyDraw(const Session& current);
void viewLoyaltyDrawHistory();

// Redemption hook, called from issueInvoice() - triggered automatically at
// invoicing time like applyDiscount(), but not automatic in what it does:
// reception is shown each of the patient's unredeemed wins with its real
// value on this invoice and picks at most one to apply, confirmed with a
// password before it commits (unrecoverable once confirmed - there is no
// un-redeem). Returns one printable line if a win was applied, empty if
// none was (no unredeemed wins, or reception chose to skip).
vector<string> applyUnredeemedWin(const Session& current, Payment& invoice, const string& patientID);

// Patient-facing "you won" notice - checked once per login, at the top of mainMenu.
void checkAndShowLoyaltyNotifications(const Session& current);

// Patient self-service: ticket count + win history for the logged-in patient.
void viewMyLoyaltyTickets(const Session& current);

#endif
