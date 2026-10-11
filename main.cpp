#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <unordered_map>

const std::string DATA_FILE = "database.dat";
const std::string INDEX_FILE = "database.idx";

// Structure to store file position information
struct FilePosition {
    std::streampos pos;
    size_t size;
    
    FilePosition() : pos(0), size(0) {}
    FilePosition(std::streampos p, size_t s) : pos(p), size(s) {}
};

// Database class for file-based storage with indexing
class FileDatabase {
private:
    // Index mapping: index -> sorted list of values with their file positions
    std::map<std::string, std::set<int>> indexCache;
    std::string dataFilename;
    std::string indexFilename;
    
    // Flag to track if data has been modified
    bool dirty;
    
    // Load index from index file
    void loadIndex() {
        if (!std::filesystem::exists(indexFilename)) {
            return;
        }
        
        std::ifstream file(indexFilename, std::ios::binary);
        if (!file) {
            return;
        }
        
        // Read index entries
        size_t entryCount;
        file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
        
        for (size_t i = 0; i < entryCount; i++) {
            // Read index string length
            uint8_t indexLen;
            file.read(reinterpret_cast<char*>(&indexLen), sizeof(indexLen));
            
            // Read index string
            std::string index(indexLen, '\0');
            file.read(&index[0], indexLen);
            
            // Read value count
            size_t valueCount;
            file.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
            
            // Read values
            for (size_t j = 0; j < valueCount; j++) {
                int value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                indexCache[index].insert(value);
            }
        }
        
        file.close();
        dirty = false;
    }
    
    // Save index to index file
    void saveIndex() {
        if (!dirty) return;
        
        std::ofstream file(indexFilename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open index file for writing: " << indexFilename << std::endl;
            return;
        }
        
        // Write entry count
        size_t entryCount = indexCache.size();
        file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
        
        // Write index entries
        for (const auto& entry : indexCache) {
            const std::string& index = entry.first;
            const std::set<int>& values = entry.second;
            
            // Write index string length (limited to 255 bytes)
            uint8_t indexLen = static_cast<uint8_t>(index.length());
            file.write(reinterpret_cast<const char*>(&indexLen), sizeof(indexLen));
            
            // Write index string
            file.write(index.c_str(), indexLen);
            
            // Write value count
            size_t valueCount = values.size();
            file.write(reinterpret_cast<const char*>(&valueCount), sizeof(valueCount));
            
            // Write values in ascending order (set is already sorted)
            for (int value : values) {
                file.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }
        }
        
        file.close();
        dirty = false;
    }
    
    // Periodically save index to prevent data loss
    void periodicSave() {
        // Save every 100 operations or when index size exceeds 10MB
        // For now, we'll save on destruction and set dirty flag
        // In a real implementation, we might want to save periodically
    }

public:
    FileDatabase(const std::string& dataFile) : dataFilename(dataFile), indexFilename(INDEX_FILE), dirty(false) {
        loadIndex();
    }
    
    ~FileDatabase() {
        saveIndex();
    }
    
    void insert(const std::string& index, int value) {
        auto& values = indexCache[index];
        auto result = values.insert(value);
        // If value was actually inserted (not duplicate)
        if (result.second) {
            dirty = true;
        }
    }
    
    void remove(const std::string& index, int value) {
        auto it = indexCache.find(index);
        if (it != indexCache.end()) {
            auto& values = it->second;
            auto valueIt = values.find(value);
            if (valueIt != values.end()) {
                values.erase(valueIt);
                // Remove the index if no values are left
                if (values.empty()) {
                    indexCache.erase(it);
                }
                dirty = true;
            }
        }
    }
    
    std::vector<int> find(const std::string& index) {
        auto it = indexCache.find(index);
        if (it != indexCache.end()) {
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