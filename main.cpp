#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <filesystem>

const std::string DATA_FILE = "database.dat";

// Database class for file-based storage
class FileDatabase {
private:
    std::map<std::string, std::set<int>> indexMap;
    std::string filename;
    
    void loadFromFile() {
        // Check if file exists
        if (!std::filesystem::exists(filename)) {
            return;
        }
        
        std::ifstream file(filename);
        if (!file) {
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string index;
            int value;
            
            if (iss >> index >> value) {
                indexMap[index].insert(value);
            }
        }
        file.close();
    }
    
    void saveToFile() {
        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Error: Cannot open file for writing: " << filename << std::endl;
            return;
        }
        
        for (const auto& pair : indexMap) {
            const std::string& index = pair.first;
            const std::set<int>& values = pair.second;
            
            for (int value : values) {
                file << index << " " << value << "\n";
            }
        }
        file.close();
    }

public:
    FileDatabase(const std::string& file) : filename(file) {
        loadFromFile();
    }
    
    void insert(const std::string& index, int value) {
        indexMap[index].insert(value);
        saveToFile();
    }
    
    void remove(const std::string& index, int value) {
        auto it = indexMap.find(index);
        if (it != indexMap.end()) {
            it->second.erase(value);
            // Remove the index if no values are left
            if (it->second.empty()) {
                indexMap.erase(it);
            }
            saveToFile();
        }
    }
    
    std::vector<int> find(const std::string& index) {
        auto it = indexMap.find(index);
        if (it != indexMap.end()) {
            return std::vector<int>(it->second.begin(), it->second.end());
        } else {
            return std::vector<int>();
        }
    }
};

int main() {
    FileDatabase db(DATA_FILE);
    
    int n;
    std::cin >> n;
    std::cin.ignore(); // Ignore the newline after n
    
    for (int i = 0; i < n; i++) {
        std::string line;
        std::getline(std::cin, line);
        
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "insert") {
            std::string index;
            int value;
            iss >> index >> value;
            db.insert(index, value);
        } else if (command == "delete") {
            std::string index;
            int value;
            iss >> index >> value;
            db.remove(index, value);
        } else if (command == "find") {
            std::string index;
            iss >> index;
            std::vector<int> results = db.find(index);
            
            if (results.empty()) {
                std::cout << "null\n";
            } else {
                for (size_t j = 0; j < results.size(); j++) {
                    if (j > 0) std::cout << " ";
                    std::cout << results[j];
                }
                std::cout << "\n";
            }
        }
    }
    
    return 0;
}