#ifndef PANINI_HPP
#define PANINI_HPP

#include <functional>
#include <algorithm>
#include <exception>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <mutex>
#include <map>

class SectionNotFound : public std::runtime_error {
public:
    SectionNotFound() : std::runtime_error("Section Not Found") {};
    SectionNotFound(const std::string& section) : std::runtime_error("Section '" + section + "' Not Found") {};
};

class PropertyNotFound : public std::runtime_error {
public:
    PropertyNotFound() : std::runtime_error("Property Not Found") {};
    PropertyNotFound(const std::string& property) : std::runtime_error("Property '" + property + "' Not Found") {};
};

class Panini{
    using ini_t = std::map<std::string, std::map<std::string, std::string>>;

public:
    Panini(void) = default;
    void read(const std::string& filename);
    static std::string& trim(std::string& str);
    const std::string& get(const std::string& section, const std::string& property) const;
    template<typename T> T getParse(const std::string& section, const std::string& property) const;

private:
    ini_t m_ini;
    std::mutex m_mu;
};

void Panini::read(const std::string &filename) {
    std::ifstream file(filename.c_str());

    if(!file.good()){
        std::ostringstream oss;
        oss << "'" << filename.c_str() << "' does not exist.";
        throw std::runtime_error(oss.str());
    }

    // acquire the private mutex whenever altering the m_ini object
    std::lock_guard<std::mutex> lock(m_mu);
    m_ini.clear();

    unsigned int lineCtr = 0;
    std::string line;
    std::string section = "";

    const char assignment = '=', comment = ';';

    while(std::getline(file, line)){
        ++lineCtr; // increment the line counter
        line = trim(line); // remove whitespace from the line
        if(line.size() == 0 || line.at(0) == comment){
            continue; // ignore blank lines and comments entirely
        }

        if(line.at(0) == '['){
            if(line.back() == ']' && line.size() > 2) {
                section = line.substr(1, line.size() - 2);
            }else{
                std::ostringstream oss;
                oss << "Invalid Section Declaration on Line " << lineCtr;
                throw std::runtime_error(oss.str());
            }
        } else {
            size_t pos;
            if((pos = line.find_first_of(assignment)) != std::string::npos) {
                std::string key(line.substr(0, pos));
                std::string val(line.substr(pos + 1));
                key = trim(key);
                val = trim(val);
                if(!key.size() || !val.size()){
                    std::ostringstream oss;
                    oss << "Neither the Key nor Value can be Empty on Line " << lineCtr;
                    throw std::runtime_error(oss.str());
                }
                if(section.size() == 0) {
                    throw std::runtime_error("No Section to Apply Settings.");
                }
                m_ini[section][key] = val;
            }else{
                std::ostringstream oss;
                oss << "Unknown Settings on Line " << lineCtr;
                throw std::runtime_error(oss.str());
            }
        }
    }
}

// remove whitespace from either side of a string
std::string& Panini::trim(std::string& str) {
    const auto space = std::function<int(int)>(::isspace);
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(space)));
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(space)).base(), str.end());
    return str;
}

// Retrieve the property value from a section. Throws a SectionNotFound or PropertyNotFound runtime_error if it does not exist
const std::string& Panini::get(const std::string& section, const std::string& property) const{
    auto sectionEntry = m_ini.find(section);
    if(sectionEntry == m_ini.end()){
        throw SectionNotFound();
    }
    auto properties = sectionEntry->second;
    auto propertyEntry = properties.find(property);
    if(propertyEntry == properties.end()){
        throw PropertyNotFound();
    }
    return propertyEntry->second;
}

template<typename T>
T Panini::getParse(const std::string& section, const std::string& property) const {
    T t;
    auto value = get(section, property);
    std::istringstream iss(value);
    iss >> t;
    if (iss.fail()) {
        std::ostringstream oss;
        oss << "Invalid Value at '" << section << ":" << property << "'.";
        throw std::runtime_error(oss.str());
    }
    return t;
}

#endif//PANINI_HPP
