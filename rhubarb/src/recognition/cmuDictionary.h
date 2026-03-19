#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "core/Phone.h"

// Parses and provides lookup for the CMU pronunciation dictionary.
// Maps English words to their Arpabet phone sequences.
class CmuDictionary {
public:
	explicit CmuDictionary(const std::filesystem::path& dictPath);

	// Returns phones for a word (case-insensitive).
	// Uses the first pronunciation variant.
	// Returns empty vector if word not found.
	std::vector<Phone> lookup(const std::string& word) const;

	bool contains(const std::string& word) const;

private:
	std::unordered_map<std::string, std::vector<Phone>> entries;
};
