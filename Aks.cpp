// RailwayReservationSystem.cpp
// Fully user-friendly console-based Railway Reservation System

#include <bits/stdc++.h>
using namespace std;

struct Passenger {
    long long ticketNo;
    string name;
    int age;
    string gender;
    string bookedAt;

    string toCSV() const {
        auto esc = [](const string &s) {
            string r = s;
            bool needQuote = r.find(',') != string::npos || r.find('"') != string::npos;
            size_t p = 0;
            while ((p = r.find('"', p)) != string::npos) { r.insert(p, "\""); p += 2; }
            if (needQuote) r = "\"" + r + "\"";
            return r;
        };
        return to_string(ticketNo) + "," + esc(name) + "," + to_string(age) + "," + esc(gender) + "," + esc(bookedAt);
    }

    static Passenger fromCSV(const string &line) {
        vector<string> fields;
        string cur;
        bool inQuotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"') {
                if (inQuotes && i+1 < line.size() && line[i+1] == '"') { cur.push_back('"'); ++i; } 
                else inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) { fields.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        fields.push_back(cur);
        Passenger p{};
        if (fields.size() >= 1) p.ticketNo = stoll(fields[0]);
        if (fields.size() >= 2) p.name = fields[1];
        if (fields.size() >= 3) p.age = stoi(fields[2]);
        if (fields.size() >= 4) p.gender = fields[3];
        if (fields.size() >= 5) p.bookedAt = fields[4];
        return p;
    }
};

class ReservationSystem {
private:
    const int TOTAL_SEATS;
    const string CONF_FILE = "confirmed.csv";
    const string WAIT_FILE = "waiting.csv";

    vector<Passenger> confirmed;
    queue<Passenger> waiting;
    long long nextTicketNo = 1;

public:
    ReservationSystem(int capacity): TOTAL_SEATS(capacity) { loadData(); }
    ~ReservationSystem() { saveData(); }

    void loadData() {
        ifstream fin(CONF_FILE);
        if (fin) {
            string line;
            while (getline(fin, line)) if (!line.empty()) confirmed.push_back(Passenger::fromCSV(line));
        }
        ifstream wfin(WAIT_FILE);
        if (wfin) {
            string line;
            while (getline(wfin, line)) if (!line.empty()) waiting.push(Passenger::fromCSV(line));
        }
    }

    void saveData() {
        ofstream fout(CONF_FILE, ios::trunc);
        for (const auto &p : confirmed) fout << p.toCSV() << "\n";
        ofstream wout(WAIT_FILE, ios::trunc);
        queue<Passenger> tmp = waiting;
        while (!tmp.empty()) { wout << tmp.front().toCSV() << "\n"; tmp.pop(); }
    }

    static string trim(const string &s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a==string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a,b-a+1);
    }

    static string nowTimestamp() {
        auto t = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(t);
        char buf[64];
        strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",localtime(&tt));
        return string(buf);
    }

    int findConfirmedIndexByTicket(long long ticket) {
        for (size_t i = 0; i < confirmed.size(); ++i) if (confirmed[i].ticketNo == ticket) return (int)i;
        return -1;
    }

    int waitingPosition(long long ticket) {
        queue<Passenger> tmp = waiting; int pos=1;
        while(!tmp.empty()){ if(tmp.front().ticketNo==ticket) return pos; tmp.pop(); pos++; }
        return -1;
    }

    bool removeFromWaiting(long long ticket) {
        bool removed=false;
        queue<Passenger> tmp;
        while(!waiting.empty()){
            Passenger p=waiting.front(); waiting.pop();
            if(p.ticketNo==ticket){ removed=true; continue; }
            tmp.push(p);
        }
        waiting=move(tmp);
        return removed;
    }

    void printPassenger(const Passenger &p,bool confirmedStatus) {
        cout << "Ticket: " << p.ticketNo 
             << " | Name: " << p.name 
             << " | Age: " << p.age 
             << " | Gender: " << p.gender 
             << " | Booked At: " << p.bookedAt 
             << " | Status: " << (confirmedStatus?"Confirmed":"Waiting") << "\n";
    }

    int readInt(string prompt){
        while(true){
            cout << prompt;
            string s; getline(cin,s); s=trim(s);
            try { return stoi(s); } catch(...) { cout << "Invalid input. Enter a number.\n"; }
        }
    }

    long long readLL(string prompt){
        while(true){
            cout << prompt;
            string s; getline(cin,s); s=trim(s);
            try { return stoll(s); } catch(...) { cout << "Invalid input. Enter a number.\n"; }
        }
    }

    string readGender(string prompt){
        while(true){
            cout << prompt;
            string g; getline(cin,g); g=trim(g);
            transform(g.begin(), g.end(), g.begin(), ::toupper);
            if(g=="M"||g=="F"||g=="O") return g;
            cout << "Invalid input. Type M/F/O.\n";
        }
    }

    void bookSeatInteractive(){
        cout << "\n--- Book a Seat ---\n";
        cout << "Confirmed seats: " << confirmed.size() << "/" << TOTAL_SEATS 
             << " | Waiting list: " << waiting.size() << "\n";

        cout << "Enter passenger name: ";
        string name; getline(cin,name); name=trim(name);
        if(name.empty()){ cout << "Name cannot be empty. Booking cancelled.\n"; return; }

        int age = readInt("Enter age (number): ");
        string gender = readGender("Enter gender (M/F/O): ");

        Passenger p; p.ticketNo=nextTicketNo++; p.name=name; p.age=age; p.gender=gender; p.bookedAt=nowTimestamp();

        if((int)confirmed.size()<TOTAL_SEATS){
            confirmed.push_back(p);
            cout << "✅ Seat confirmed! Ticket No: " << p.ticketNo << "\n";
        } else {
            waiting.push(p);
            cout << "⚠️ No seats available. Added to waiting list. Ticket No: " << p.ticketNo 
                 << " | Position: " << waitingPosition(p.ticketNo) << "\n";
        }
    }

    void cancelBookingInteractive(){
        cout << "\n--- Cancel Booking ---\n";
        long long t = readLL("Enter ticket number to cancel: ");
        int idx=findConfirmedIndexByTicket(t);
        if(idx!=-1){
            Passenger removed=confirmed[idx];
            confirmed.erase(confirmed.begin()+idx);
            cout << "✅ Cancelled ticket " << removed.ticketNo << " (" << removed.name << ")\n";
            if(!waiting.empty()){
                Passenger promote=waiting.front(); waiting.pop();
                promote.bookedAt=nowTimestamp();
                confirmed.push_back(promote);
                cout << "🔄 Promoted from waiting: Ticket " << promote.ticketNo << "\n";
            }
        } else if(removeFromWaiting(t)){
            cout << "✅ Removed from waiting list: Ticket " << t << "\n";
        } else {
            cout << "❌ Ticket not found.\n";
        }
    }

    void listAll(){
        cout << "\n--- Confirmed Bookings ---\n";
        if(confirmed.empty()) cout << "(none)\n";
        for(auto&p:confirmed) printPassenger(p,true);

        cout << "\n--- Waiting List ---\n";
        if(waiting.empty()) cout << "(none)\n";
        queue<Passenger> tmp=waiting; int pos=1;
        while(!tmp.empty()){ cout << "[" << pos++ << "] "; printPassenger(tmp.front(),false); tmp.pop(); }
    }

    void showMenu(){
        cout << "\n=== Railway Reservation System ===\n"
             << "1. Book seat\n"
             << "2. Cancel booking\n"
             << "3. List all bookings\n"
             << "0. Exit\n";
    }

    void runConsole(){
        while(true){
            showMenu();
            int choice = readInt("Choose an option: ");
            if(choice==1) bookSeatInteractive();
            else if(choice==2) cancelBookingInteractive();
            else if(choice==3) listAll();
            else if(choice==0){ cout<<"Exiting..."; break; }
            else cout << "Invalid option. Try again.\n";
            cout << "Press Enter to continue..."; string dummy; getline(cin,dummy);
        }
    }
};

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    ReservationSystem app(10);
    app.runConsole();
    return 0;
}
