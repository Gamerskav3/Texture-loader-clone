#pragma once
#include <Geode/Geode.hpp>
#include <filesystem>
#include <string>
#include <vector>

struct Pack {
	std::string id;
	std::string name;
	std::string version;
	std::filesystem::path path;
	bool enabled = false;
};

namespace packs {
	// <config dir>/packs : drop pack folders or .zip files here
	std::filesystem::path packsDir();
	// all packs found, in saved priority order (top = highest priority)
	std::vector<Pack> scan();
	// saves order + enabled state
	void save(std::vector<Pack> const& list);
	// adds the enabled packs to the game's search paths (call at startup)
	void applyEnabled();
}
