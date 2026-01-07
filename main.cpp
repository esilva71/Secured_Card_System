#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <limits>
#include <regex>
#include <ctime>

// ----------- model classes 

struct Card {
    std::string id;
    int clearanceLevel{};
};

struct User {
    std::string id;
    std::string name;
    std::string email;
    std::string phone;
    Card card;
};

struct Admin {
    std::string id;
    std::string password;
    std::string name;
    std::string email;
    std::string phone;
    Card card;
};

struct AccessLogEntry {
    std::string userId;
    std::string userName;
    std::string timeStr;
    bool authorized{};
};

struct Floor {
    std::string id;
    std::string name;
    int clearanceLevel{};
    std::vector<AccessLogEntry> accessHistory;
};

// ---------- Support functions 

std::string timeStamp() {
    std::time_t t = std::time(nullptr);
    char buf[64]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

bool validatePassword(const std::string& p) {
    if (p.size() < 8) return false;
    bool upper=false, lower=false, digit=false, special=false;
    for (char c : p) {
        if (std::isupper(static_cast<unsigned char>(c))) upper = true;
        else if (std::islower(static_cast<unsigned char>(c))) lower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) digit = true;
        else special = true;
    }
    return upper && lower && digit && special;
}

bool validateEmail(const std::string& email) {
    //basic email validation using a regular expression
    static const std::regex pattern(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)");
    return std::regex_match(email, pattern);
}

bool validatePhone(const std::string& phone) {
    // 07XXXXXXXX or +467XXXXXXXX
    static const std::regex pattern(R"(^07\d{8}$|^\+467\d{8}$)");
    return std::regex_match(phone, pattern);
}

// split CSV line with comma
std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        parts.push_back(item);
    }
    return parts;
}

// --------------------- Persistence 

bool loadUsers(const std::string& filename, std::vector<User>& users) {
    std::ifstream file(filename);
    if (!file) return false;

    std::string line;
    if (!std::getline(file, line)) return true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto cols = splitCSV(line);
        //User: ID,Name,Email,Phone,CardID,CardClearance
        if (cols.size() < 6) continue;
        User u;
        u.id      = cols[0];
        u.name    = cols[1];
        u.email   = cols[2];
        u.phone   = cols[3];
        u.card.id = cols[4];
        u.card.clearanceLevel = std::stoi(cols[5]);
        users.push_back(u);
    }
    return true;
}

bool saveUsers(const std::string& filename, const std::vector<User>& users) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "ID,Name,Email,Phone,CardID,CardClearance\n";
    for (const auto& u : users) {
        file << u.id << ','
             << u.name << ','
             << u.email << ','
             << u.phone << ','
             << u.card.id << ','
             << u.card.clearanceLevel << '\n';
    }
    return true;
}

bool loadAdmins(const std::string& filename, std::vector<Admin>& admins) {
    std::ifstream file(filename);
    if (!file) return false;
    std::string line;
    if (!std::getline(file, line)) return true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto cols = splitCSV(line);
        //Admin: ID,Password,Name,Email,Phone,CardID,CardClearance
        if (cols.size() < 7) continue;
        Admin a;
        a.id       = cols[0];
        a.password = cols[1];
        a.name     = cols[2];
        a.email    = cols[3];
        a.phone    = cols[4];
        a.card.id  = cols[5];
        a.card.clearanceLevel = std::stoi(cols[6]);
        admins.push_back(a);
    }
    return true;
}

bool saveAdmins(const std::string& filename, const std::vector<Admin>& admins) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "ID,Password,Name,Email,Phone,CardID,CardClearance\n";
    for (const auto& a : admins) {
        file << a.id << ','
             << a.password << ','
             << a.name << ','
             << a.email << ','
             << a.phone << ','
             << a.card.id << ','
             << a.card.clearanceLevel << '\n';
    }
    return true;
}

bool loadFloors(const std::string& filename, std::vector<Floor>& floors) {
    std::ifstream file(filename);
    if (!file) return false;
    std::string line;
    if (!std::getline(file, line)) return true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto cols = splitCSV(line);
        // Floor: ID,Name,ClearanceLevel
        if (cols.size() < 3) continue;
        Floor f;
        f.id = cols[0];
        f.name = cols[1];
        f.clearanceLevel = std::stoi(cols[2]);
        floors.push_back(f);
    }
    return true;
}

bool saveFloors(const std::string& filename, const std::vector<Floor>& floors) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "ID,Name,ClearanceLevel\n";
    for (const auto& f : floors) {
        file << f.id << ','
             << f.name << ','
             << f.clearanceLevel << '\n';
    }
    return true;
}

// ---------- Search functions

User* findUser(std::vector<User>& users, const std::string& key) {
    for (auto& u : users) {
        if (u.id == key || u.name == key) return &u;
    }
    return nullptr;
}

Admin* findAdmin(std::vector<Admin>& admins, const std::string& id) {
    for (auto& a : admins) {
        if (a.id == id) return &a;
    }
    return nullptr;
}

Floor* findFloor(std::vector<Floor>& floors, const std::string& key) {
    for (auto& f : floors) {
        if (f.id == key || f.name == key) return &f;
    }
    return nullptr;
}

// --------- User services 

void userAccess(User& user, std::vector<Floor>& floors) {
        std::cout << "\nAvailable floors:\n";
    for (const auto& f : floors) {
        std::cout << " - " << f.id << " : " << f.name
                  << " (clearance " << f.clearanceLevel << ")\n";
    }
    std::cout << "Enter floor Id or Floor name you are requesting to access: ";
    std::string key;
    std::getline(std::cin, key);
    Floor* floor = findFloor(floors, key);
    if (!floor) {
        std::cout << "Floor not found.\n";
        return;
    }
    bool authorized = user.card.clearanceLevel >= floor->clearanceLevel;
    std::string timeStr = timeStamp();
    floor->accessHistory.push_back({user.id, user.name, timeStr, authorized});
    if (authorized) {
        std::cout << "Access granted to floor " << floor->name
                  << " at " << timeStr << ".\n";
    } else {
        std::cout << "Access denied to floor " << floor->name
                  << " at " << timeStr << ".\n";
    }
}

void showUser(User& user) {
    std::cout << "\nYour information:\n";
    std::cout << "ID: " << user.id << "\n";
    std::cout << "Name: " << user.name << "\n";
    std::cout << "Email: " << user.email << "\n";
    std::cout << "Phone: " << user.phone << "\n";
    std::cout << "Card ID: " << user.card.id << "\n";
    std::cout << "Card clearance: " << user.card.clearanceLevel << "\n";
}

void editUser(User& user) {
    std::string input;

    std::cout << "New name (leave empty to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) user.name = input;

    std::cout << "New email (leave empty to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        if (validateEmail(input)) {
            user.email = input;
        } else {
            std::cout << "Invalid email; unchanged.\n";
        }
    }

    std::cout << "New phone (leave empty to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        if (validatePhone(input)) {
            user.phone = input;
        } else {
            std::cout << "Invalid phone; unchanged.\n";
        }
    }
}

int getChoice(int min, int max) {
    int choice;
    while (!(std::cin >> choice) || choice < min || choice > max) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid input. Enter a number between " << min << " and " << max << ": ";
    }
    std::cin.ignore();
    return choice;
}

void userMenu(User& user, std::vector<Floor>& floors) {
    while (true) {
        std::cout << "\nUser menu for " << user.name << std::endl;
        std::cout << "[1] List floors / access floor" << std::endl;
        std::cout << "[2] Show / edit personal info" << std::endl;
        std::cout << "[3] Go Back" << std::endl;
        std::cout << "Choose an option: ";
        int choice = getChoice(1, 3);

        if (choice == 1) {
            userAccess(user, floors);
        } else if (choice == 2) {
            showUser(user);
            std::cout << "\n[1] Change info" << std::endl;
            std::cout << "[0] Back" << std::endl;
            std::cout << "Choose an option: ";
            int c = getChoice(0, 1);

            if (c == 1) {
                editUser(user);
            }
        } else if (choice == 3) {
            break;
        }
    }
}

// ---------admin menus

void floorHistory(Floor& floor) {
    std::cout << "\nAccess history for floor " << floor.name << ":\n";
    if (floor.accessHistory.empty()) {
        std::cout << "No entries.\n";
        return;
    }
    for (const auto& e : floor.accessHistory) {
        std::cout << e.timeStr << " - " << e.userId << " (" << e.userName << ") -> " << (e.authorized ? "AUTHORIZED" : "DENIED") << "\n";
    }
}

void editFloor(Floor& floor) {
    std::string input;
    std::cout << "New name (empty to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) floor.name = input;

    std::cout << "New clearance level (empty to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            floor.clearanceLevel = std::stoi(input);
        } catch (...) {
            std::cout << "Invalid number; unchanged.\n";
        }
    }
}

void listUsers(const std::vector<User>& users) {
    std::cout << "\nUsers:\n";
    for (const auto& u : users) {
        std::cout << u.id << " - " << u.name
                  << " (card " << u.card.id
                  << ", clearance " << u.card.clearanceLevel << ")\n";
    }
}

void deleteUser(std::vector<User>& users, const std::string& key) {
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->id == key || it->name == key) {
            users.erase(it);
            std::cout << "User (and their card) deleted.\n";
            return;
        }
    }
    std::cout << "User not found.\n";
}

void createUser(std::vector<User>& users) {
    User u;
    std::cout << "New user ID: ";
    std::getline(std::cin, u.id);

    // Ensure that the ID is unique
    if (findUser(users, u.id)) {
        std::cout << "User with that ID already exists.\n";
        return;
    }

    std::cout << "Name: ";
    std::getline(std::cin, u.name);

    std::cout << "Email: ";
    std::getline(std::cin, u.email);
    if (!validateEmail(u.email)) {
        std::cout << "Invalid email.\n";
        return;
    }

    std::cout << "Phone: ";
    std::getline(std::cin, u.phone);
    if (!validatePhone(u.phone)) {
        std::cout << "Invalid phone.\n";
        return;
    }

    std::cout << "Card ID: ";
    std::getline(std::cin, u.card.id);

    std::cout << "Card clearance level (int): ";
    std::string tmp;
    std::getline(std::cin, tmp);
    try {
        u.card.clearanceLevel = std::stoi(tmp);
    } catch (...) {
        std::cout << "Invalid number.\n";
        return;
    }

    users.push_back(u);
    std::cout << "User created.\n";
}

void adminMenu(Admin& admin, std::vector<User>& users, std::vector<Floor>& floors) {
    while (true) {
        std::cout << "\nAdmin menu (" << admin.name << "):\n";
        std::cout << "[1] List floors / manage floor\n";
        std::cout << "[2] List / manage users\n";
        std::cout << "[3] Create new user\n";
        std::cout << "[4] Change Admin password\n";
        std::cout << "[5] Go Back\n";
        std::cout << "Choose an option: ";
        int choice = getChoice(1, 5);

        if (choice == 1) {
                std::cout << "\nAvailable floors:\n";
                for (const auto& f : floors) {
                    std::cout << " - " << f.id << " : " << f.name
                    << " (clearance " << f.clearanceLevel << ")\n";
                }
            std::cout << "Choose floor id or name (empty = back): ";
            std::string key;
            std::getline(std::cin, key);
            if (key.empty()) continue;
            Floor* floor = findFloor(floors, key);
            if (!floor) {
                std::cout << "Floor not found.\n";
                continue;
            }
            std::cout << "[1] Show access history" << std::endl;
            std::cout << "[2] Change floor info" << std::endl;
            std::cout << "[0] Back" << std::endl;
            std::cout << "Choose an option: ";
            int c = getChoice(0, 2);
            if (c == 1) floorHistory(*floor);
            else if (c == 2) editFloor(*floor);
        }
        else if (choice == 2) {
            listUsers(users);
            std::cout << "Choose user by ID or name (empty = back): ";
            std::string key;
            std::getline(std::cin, key);
            if (key.empty()) continue;
            User* u = findUser(users, key);
            if (!u) {
                std::cout << "User not found.\n";
                continue;
            }
            std::cout << "[1] Change user info" << std::endl;
            std::cout << "[2] Delete user" << std::endl;
            std::cout << "[0] Back" << std::endl;
            std::cout << "Choose an option: ";

            int c = getChoice(0, 2);

            if (c == 1) editUser(*u);
            else if (c == 2) deleteUser(users, key);
        }
        else if (choice == 3) {
            createUser(users);
        }
        else if (choice == 4) {
            std::cout << "\nHello (" << admin.name << "):\n";
            std::cout << "Enter new password (empty = back): ";
            std::string passwd;
            std::getline(std::cin, passwd);
            if (passwd.empty()) continue;
            if (validatePassword(passwd)) {
                admin.password = passwd;
                std::cout << "Password changed successfully.\n";
            } else {
                std::cout << "Password must be at least 8 characters long and include uppercase, lowercase, digit, and special character.\n";
            }
        }
        else if (choice == 5) {
            break;
        }
    }
}

// -------------- main


int main() {
    std::vector<User> users;
    std::vector<Admin> admins;
    std::vector<Floor> floors;
    

    // csv file names
    const std::string usersFile  = "users.csv";
    const std::string adminsFile = "admins.csv";
    const std::string floorsFile = "floors.csv";

    // Load data
    loadUsers(usersFile, users);
    loadAdmins(adminsFile, admins);
    loadFloors(floorsFile, floors);

    while (true) {
        std::cout << "\nSecure Card System\n";
        std::cout << "[1] User Login" << std::endl;
        std::cout << "[2] Admin Login" << std::endl;
        std::cout << "[0] Exit" << std::endl;
        std::cout << "Choose an option: ";
        int role = getChoice(0, 2);
        if (role == 0) {
            break;
        } else if (role == 1) {
            std::cout << "Enter User ID or Full name: ";
            std::string key;
            std::getline(std::cin, key);
            User* user = findUser(users, key);
            if (!user) {
                std::cout << "User not found.\n";
            } else {
                userMenu(*user, floors);
            }
        } else if (role == 2) {
            std::cout << "Admin ID: ";
            std::string id;
            std::getline(std::cin, id);
            Admin* admin = findAdmin(admins, id);
            if (!admin) {
                std::cout << "Admin not found.\n";
                continue;
            }
            std::cout << "Password: ";
            std::string pw;
            std::getline(std::cin, pw);
            if (pw != admin->password) {
                std::cout << "Wrong password.\n";
                continue;
            }
            adminMenu(*admin, users, floors);
        }
    }

    // Save data when exit
    saveUsers(usersFile, users);
    saveAdmins(adminsFile, admins);
    saveFloors(floorsFile, floors);

    std::cout << "Goodbye.\n";
    return 0;
}
