#include <iostream>
#include <fstream>
#include <optional>
#include <tuple>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <exception>
#include <cassert>

struct Person {
    std::string mSecondName;
    std::string mFirstName;
    std::optional<std::string> mPatronymic = nullptr;
    bool operator < (const Person& right) const;
    bool operator == (const Person& right) const;
};

std::ostream& operator << (std::ostream& strm, const Person& p) {
    strm << std::setw(12) << p.mSecondName << " " << std::setw(9) << p.mFirstName << std::setw(15) << p.mPatronymic.value_or("");
    return strm;
}

bool Person::operator<(const Person &right) const {
    return  tie(mSecondName, mFirstName, mPatronymic) < tie(right.mSecondName, right.mFirstName, right.mPatronymic);
}

bool Person::operator==(const Person &right) const {
    return  tie(mSecondName, mFirstName, mPatronymic) == tie(right.mSecondName, right.mFirstName, right.mPatronymic);
}

struct PhoneNumber {
    unsigned short int mCountry;
    unsigned short int mCity;
    std::string mNumber;
    std::optional<int> mAdd;
    bool operator < (const PhoneNumber& right) const;
    bool operator == (const PhoneNumber& right) const;
};

std::ostream& operator << (std::ostream &strm, const PhoneNumber &p) {
    strm << "+" << p.mCountry << "(" << p.mCity << ")" << p.mNumber;
    return strm;
}

bool PhoneNumber::operator<(const PhoneNumber &right) const {
    return tie(mCountry, mCity, mNumber, mAdd) < tie(right.mCountry, right.mCity, right.mNumber, right.mAdd);
}

bool PhoneNumber::operator==(const PhoneNumber &right) const {
    return tie(mCountry, mCity, mNumber, mAdd) == tie(right.mCountry, right.mCity, right.mNumber, right.mAdd);
}

class PhoneBook {
    std::vector<std::pair<Person, PhoneNumber>> mData;
public:
    PhoneBook(std::ifstream &file) {
        std::string recordString;
        std::stringstream buf;
        std::vector<std::string> fieldsVec;
        std::string fieldStr;
        while (!file.eof()) {
            getline(file, recordString);
            buf << recordString;
            while (!buf.eof()) {
                buf >> fieldStr;
                fieldsVec.push_back(fieldStr);
            }
            if (fieldsVec.size() < 5) {
                continue;
            }
            if (!isalpha(fieldsVec[0][0]) ||
                !isalpha(fieldsVec[1][0]) ||
                !isdigit(fieldsVec[3][0]) ||
                !isdigit(fieldsVec[4][0]) ||
                (!(isalpha(fieldsVec[2][0]) || isdigit(fieldsVec[2][0]))))
                continue;
            int phoneStart = isalpha(fieldsVec[2][0]) ? 3 : 2;
            Person prsn = {fieldsVec[0], fieldsVec[1], (isalpha(fieldsVec[2][0]) ? std::optional<std::string>(fieldsVec[2]) : std::nullopt)};
            PhoneNumber nmbr = {static_cast<unsigned short>((stoi(fieldsVec[phoneStart]))), static_cast<unsigned short>(stoi(fieldsVec[phoneStart+1])), fieldsVec[phoneStart+2]};
            mData.push_back(std::make_pair(prsn, nmbr));
            std::vector<std::string>().swap(fieldsVec);
            buf.clear();
        }
    }
    
    void SortByPhone() {
        sort(mData.begin(), mData.end(), [](const std::pair<Person, PhoneNumber> &l, const std::pair<Person, PhoneNumber> &r)
        {return l.second < r.second;});
    }
    
    void SortByName() {
        sort(mData.begin(), mData.end());
    }
    
    std::pair<std::string, PhoneNumber> GetPhoneNumber(const std::string &name) const {
        PhoneNumber foundNumber;
        for (const auto &rec : mData) {
            if (rec.first.mSecondName == name) {
                if (!foundNumber.mNumber.empty()) {
                    return std::make_pair("found more than 1", foundNumber);
                }
                foundNumber = rec.second;
            }
        }
        return std::make_pair(foundNumber.mNumber.empty() ? "not found" : "", foundNumber);
    }
    
    bool ChangePhoneNumber(const Person &prsn, const PhoneNumber &phone) {
        for (auto &rec : mData) {
            if (rec.first == prsn) {
                rec.second = phone;
                return true;
            }
        }
        return false;
    }
    friend std::ostream& operator << (std::ostream& strm, const PhoneBook& ob);
};

std::ostream& operator << (std::ostream& strm, const PhoneBook& ob) {
    for (const auto &rec : ob.mData)
        strm << rec.first << "\t" << rec.second << std::endl;
    return strm;
}

void testSortByName(PhoneBook &book) {
    book.SortByName();
}

void testSortByPhone(PhoneBook &book) {
    book.SortByPhone();
}

template <typename TestFunc>
void RunTest(TestFunc func, PhoneBook &book, const std::string& test_name) {
    try {
        func(book);
        std::cerr << test_name << " OK" << std::endl;
    }
    catch (std::runtime_error& e) {
             std::cerr << test_name << " FAIL: " << e.what() << std::endl;
    }
}

int main() {
    std::ifstream file("PhoneBook.txt");
    if (!file.is_open()) {
        std::cerr << "Can't open file." << std::endl;
        return 1;
    }
    PhoneBook book(file);
    file.close();
    RunTest(testSortByName, book, "Sort by name");
    RunTest(testSortByPhone, book, "Sort by phone");

//    std::cout << book;
    return 0;
}
