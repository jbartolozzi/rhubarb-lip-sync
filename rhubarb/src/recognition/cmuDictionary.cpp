#include "cmuDictionary.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "logging/logging.h"

using std::string;
using std::vector;
using std::ifstream;
using std::istringstream;
using boost::optional;

static string toLower(string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return s;
}

// Strip stress markers from Arpabet phone names (e.g., "AH0" -> "AH", "IY1" -> "IY").
// The vendored CMU dict doesn't use stress markers, but this handles it if they appear.
static string stripStress(const string& phone) {
	if (phone.empty()) return phone;
	char last = phone.back();
	if (last >= '0' && last <= '2') {
		return phone.substr(0, phone.size() - 1);
	}
	return phone;
}

CmuDictionary::CmuDictionary(const std::filesystem::path& dictPath) {
	ifstream file(dictPath);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open CMU dictionary: " + dictPath.string());
	}

	string line;
	int entryCount = 0;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == ';') continue;

		istringstream iss(line);
		string word;
		iss >> word;
		if (word.empty()) continue;

		// Skip alternate pronunciations like "word(2)", "word(3)"
		if (word.back() == ')' && word.size() > 3) {
			auto parenPos = word.find('(');
			if (parenPos != string::npos) continue;
		}

		string wordLower = toLower(word);

		// Parse phones
		vector<Phone> phones;
		string phoneStr;
		bool valid = true;
		while (iss >> phoneStr) {
			phoneStr = stripStress(phoneStr);
			optional<Phone> phone = PhoneConverter::get().tryParse(phoneStr);
			if (phone) {
				phones.push_back(*phone);
			} else {
				valid = false;
				break;
			}
		}

		if (valid && !phones.empty() && entries.find(wordLower) == entries.end()) {
			entries[wordLower] = std::move(phones);
			++entryCount;
		}
	}

	logging::debugFormat("Loaded CMU dictionary with {} entries from {}",
		entryCount, dictPath.string());
}

vector<Phone> CmuDictionary::lookup(const string& word) const {
	auto it = entries.find(toLower(word));
	if (it != entries.end()) {
		return it->second;
	}
	return {};
}

bool CmuDictionary::contains(const string& word) const {
	return entries.find(toLower(word)) != entries.end();
}
