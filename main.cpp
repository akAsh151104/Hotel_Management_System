#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <limits>
#include <windows.h>
#include <mysql.h>
using namespace std;

// ---------- DB Credentials ----------
const char* HOST = "localhost";
const char* USER = "root";
const char* PW   = "Akash@Ankush"; // Change to your DB password
const char* DB   = "hotel_management";

// ---------- Utility ----------
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void pauseScreen() {
    setColor(8);
    cout << "\n\n   Press Enter to continue...";
    setColor(7);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void showHeader(const string& title, int color = 14) {
    system("cls");
    setColor(color);
    cout << "\n+----------------------------------------------------------+\n";
    cout << "¦" << setw((58 + title.size()) / 2) << title
         << setw(58 - (58 + title.size()) / 2 + 1) << "¦\n";
    cout << "+----------------------------------------------------------+\n";
    setColor(7);
}

void showDivider() {
    setColor(8);
    cout << string(60, '-') << "\n";
    setColor(7);
}

// ---------- DB Manager Class ----------
class DBManager {
public:
    MYSQL* conn;
    DBManager() { conn = mysql_init(0); }
    bool connect() {
        conn = mysql_real_connect(conn, HOST, USER, PW, DB, 0, NULL, 0);
        return conn != NULL;
    }
    bool exec(const string& query) {
        return mysql_query(conn, query.c_str()) == 0;
    }
    MYSQL_RES* fetch(const string& query) {
        if (mysql_query(conn, query.c_str())) return NULL;
        return mysql_store_result(conn);
    }
    string error() { return mysql_error(conn); }
    void close() { mysql_close(conn); }
};

// ---------- Guest Portal ----------
class GuestPortal {
    DBManager& db;
public:
    GuestPortal(DBManager& db) : db(db) {}

    void bookRoom() {
        showHeader("BOOK A ROOM");
        int roomNo, days;
        string name, address, phone, rtype;
        long long cost = 0;

        cout << "   ? Room No (1-100): "; cin >> roomNo;
        cout << "   ? Name    : "; cin.ignore(); getline(cin, name);
        cout << "   ? Address : "; getline(cin, address);
        cout << "   ? Phone   : "; getline(cin, phone);

        cout << "\n   ?? 1. Deluxe (10000/day)\n   ?? 2. Executive (12500/day)\n   ?? 3. Presidential (15000/day)\n";
        cout << "   ? Choose Room Type: "; int choice; cin >> choice;
        cout << "   ? No. of Days     : "; cin >> days;

        if (choice == 1) { rtype = "Deluxe"; cost = 10000 * days; }
        else if (choice == 2) { rtype = "Executive"; cost = 12500 * days; }
        else if (choice == 3) { rtype = "Presidential"; cost = 15000 * days; }
        else {
            setColor(12); cout << "   ? Invalid room type."; setColor(7); pauseScreen(); return;
        }

        stringstream ss;
        ss << "INSERT INTO hotel_bookings VALUES(" << roomNo << ", '" << name << "', '" << address
           << "', '" << phone << "', " << days << ", " << cost << ", '" << rtype << "', 0, 'Occupied', DEFAULT)";

        if (db.exec(ss.str())) {
            setColor(10); cout << "   ? Room booked successfully. Total: Rs." << cost << endl;
        } else {
            setColor(12); cout << "   ? Error: " << db.error();
        }
        setColor(7);
        pauseScreen();
    }

    void viewBooking() {
        showHeader("VIEW MY BOOKING");
        int roomNo;
        cout << "   ? Enter Room No: "; cin >> roomNo;
        stringstream ss;
        ss << "SELECT * FROM hotel_bookings WHERE room_no = " << roomNo;
        MYSQL_RES* res = db.fetch(ss.str());
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            showDivider();
            cout << "\n   Name    : " << row[1]
                 << "\n   Phone   : " << row[3]
                 << "\n   Type    : " << row[6]
                 << "\n   Days    : " << row[4]
                 << "\n   Total   : Rs." << row[5]
                 << "\n   Status  : " << row[8];
        } else {
            setColor(12); cout << "   ? No booking found."; setColor(7);
        }
        mysql_free_result(res);
        pauseScreen();
    }

    void cancelBooking() {
        showHeader("CANCEL BOOKING");
        int roomNo;
        cout << "   ? Enter Room No to Cancel: "; cin >> roomNo;
        stringstream ss;
        ss << "DELETE FROM hotel_bookings WHERE room_no = " << roomNo;
        if (db.exec(ss.str())) {
            setColor(10); cout << "   ? Booking cancelled.";
        } else {
            setColor(12); cout << "   ? Error: " << db.error();
        }
        setColor(7);
        pauseScreen();
    }

    void orderFood() {
        showHeader("ORDER FOOD");
        int roomNo, choice, qty;
        cout << "   ? Room No: "; cin >> roomNo;
        cout << "   ?? 1. Breakfast (Rs.500)\n   ?? 2. Lunch (Rs.1000)\n   ?? 3. Dinner (Rs.1200)\n";
        cout << "   ? Choice: "; cin >> choice;
        cout << "   ? Quantity: "; cin >> qty;

        long long amt = (choice == 1) ? 500 * qty : (choice == 2) ? 1000 * qty : 1200 * qty;
        string item = (choice == 1) ? "Breakfast" : (choice == 2) ? "Lunch" : "Dinner";

        stringstream bill, log;
        bill << "UPDATE hotel_bookings SET cost = cost + " << amt << ", pay = pay + " << amt
             << " WHERE room_no = " << roomNo;
        log << "INSERT INTO food_orders(room_no, items, amount) VALUES("
            << roomNo << ", '" << item << "', " << amt << ")";

        db.exec(bill.str());
        db.exec(log.str());

        setColor(10); cout << "   ? Food order added to bill."; setColor(7);
        pauseScreen();
    }

    void guestMenu() {
        int ch;
        do {
            showHeader("GUEST PORTAL", 11);
            cout << "   ?? 1. Book Room\n   ?? 2. View Booking\n   ?? 3. Cancel Booking\n";
            cout << "   ?? 4. Order Food\n   ?? 5. Back\n";
            cout << "\n   ? Enter choice: "; cin >> ch;
            switch (ch) {
                case 1: bookRoom(); break;
                case 2: viewBooking(); break;
                case 3: cancelBooking(); break;
                case 4: orderFood(); break;
                case 5: return;
                default: setColor(12); cout << "   ? Invalid choice."; setColor(7); pauseScreen();
            }
        } while (true);
    }
};

// ---------- Admin Portal ----------
class AdminPortal {
    DBManager& db;
public:
    AdminPortal(DBManager& db) : db(db) {}

    bool login() {
        showHeader("ADMIN LOGIN", 13);
        string u, p;
        cout << "   Username: "; cin >> u;
        cout << "   Password: "; cin >> p;
        stringstream ss;
        ss << "SELECT * FROM users WHERE username='" << u << "' AND password_hash=SHA2('" << p << "',256)";
        MYSQL_RES* res = db.fetch(ss.str());
        bool ok = res && mysql_num_rows(res) > 0;
        mysql_free_result(res);
        return ok;
    }

    void checkout() {
        showHeader("CHECKOUT GUEST");
        int r; cout << "   ? Room No: "; cin >> r;
        stringstream ss;
        ss << "SELECT name, cost FROM hotel_bookings WHERE room_no = " << r;
        MYSQL_RES* res = db.fetch(ss.str());
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            setColor(12); cout << "   ? Not found."; setColor(7);
            pauseScreen(); return;
        }

        cout << "\n   Name : " << row[0] << "\n   Total: Rs." << row[1];
        mysql_free_result(res);

        cout << "\n\n   Confirm checkout (y/n): ";
        char ch; cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            stringstream del;
            del << "DELETE FROM hotel_bookings WHERE room_no = " << r;
            db.exec(del.str());
            setColor(10); cout << "   ? Checked out successfully."; setColor(7);
        }
        pauseScreen();
    }

    void viewAll() {
        showHeader("ALL BOOKINGS");
        MYSQL_RES* res = db.fetch("SELECT room_no,name,phone,rtype,days,cost FROM hotel_bookings ORDER BY room_no");
        MYSQL_ROW row;
        cout << left << setw(8) << "Room" << setw(15) << "Name"
             << setw(15) << "Phone" << setw(15) << "Type"
             << setw(8) << "Days" << setw(10) << "Cost\n";
        showDivider();
        while ((row = mysql_fetch_row(res))) {
            cout << left << setw(8) << row[0] << setw(15) << row[1]
                 << setw(15) << row[2] << setw(15) << row[3]
                 << setw(8) << row[4] << setw(10) << row[5] << "\n";
        }
        mysql_free_result(res);
        pauseScreen();
    }

    void adminMenu() {
        if (!login()) {
            setColor(12); cout << "   ? Login Failed."; setColor(7); pauseScreen(); return;
        }

        int ch;
        do {
            showHeader("ADMIN PORTAL", 13);
            cout << "   ?? 1. View All Bookings\n   ?? 2. Checkout Guest\n   ?? 3. Logout\n";
            cout << "\n   ? Enter choice: "; cin >> ch;
            switch (ch) {
                case 1: viewAll(); break;
                case 2: checkout(); break;
                case 3: return;
                default: setColor(12); cout << "   ? Invalid choice."; setColor(7); pauseScreen();
            }
        } while (true);
    }
};

// ---------- Main ----------
int main() {
    DBManager db;
    if (!db.connect()) {
        setColor(12);
        cout << "   ? DB Connection Failed: " << db.error() << "\n";
        setColor(7);
        return 1;
    }

    GuestPortal guest(db);
    AdminPortal admin(db);

    int choice;
    do {
        showHeader("HOTEL MANAGEMENT SYSTEM", 11);
        cout << "   ?? 1. Guest Portal\n   ?? 2. Admin Portal\n   ?? 3. Exit\n";
        cout << "\n   ? Enter choice: "; cin >> choice;
        switch (choice) {
            case 1: guest.guestMenu(); break;
            case 2: admin.adminMenu(); break;
            case 3:
                setColor(10);
                cout << "\n   ? Thank you for using the system!\n";
                setColor(7);
                break;
            default: setColor(12); cout << "   ? Invalid choice."; setColor(7); pauseScreen();
        }
    } while (choice != 3);

    db.close();
    return 0;
}

