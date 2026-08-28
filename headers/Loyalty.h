#ifndef LOYALTY_H
#define LOYALTY_H

#include <string>
#include <vector>
#include "Session.h"
#include "Payment.h" // for the Payment struct (redemption mutates one) and the ServiceItem catalog

using namespace std;

// Named Constants
const int MAX_DRAW_WINNERS = 10; // sanity cap on how many winners staff can draw in one ceremony

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

// One drawn outcome. Created the instant a ticket wins; auto-redeemed the next
// time Reception issues that patient any invoice (see issueInvoice()).
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

// Auto-redemption hook, called from issueInvoice() - mirrors applyDiscount().
// Applies every one of this patient's unredeemed wins (oldest first), marks
// them redeemed against `invoice`, and returns one printable line per win
// applied (empty vector = nothing to apply, the common case).
vector<string> applyUnredeemedWin(Payment& invoice, const string& patientID);

// Patient-facing "you won" notice - checked once per login, at the top of mainMenu.
void checkAndShowLoyaltyNotifications(const Session& current);

// Patient self-service: ticket count + win history for the logged-in patient.
void viewMyLoyaltyTickets(const Session& current);

#endif
