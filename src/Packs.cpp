#include "Packs.hpp"
#include <algorithm>

using namespace geode::prelude;
namespace fs = std::filesystem;

namespace {
	// If the zip had a wrapper folder, descend into it
	fs::path resolveRoot(fs::path const& p) {
		std::error_code ec;
		if (fs::exists(p / "pack.json", ec)) return p;
		fs::path found;
		int dirs = 0;
		for (auto& e : fs::directory_iterator(p, ec)) {
			if (e.is_directory(ec)) {
				dirs++;
				found = e.path();
			}
		}
		if (dirs == 1) return found;
		return p;
	}

	void addPack(std::vector<Pack>& out, fs::path const& root, std::string const& fallbackName) {
		Pack pack;
		pack.path = root;
		pack.id = fallbackName;
		pack.name = fallbackName;
		auto json = file::readJson(root / "pack.json");
		if (json) {
			auto v = json.unwrap();
			auto id = v["id"].asString().unwrapOr("");
			auto name = v["name"].asString().unwrapOr("");
			auto version = v["version"].asString().unwrapOr("");
			if (!id.empty()) pack.id = id;
			if (!name.empty()) pack.name = name;
			pack.version = version;
		}
		out.push_back(std::move(pack));
	}
}

fs::path packs::packsDir() {
	return Mod::get()->getConfigDir() / "packs";
}

std::vector<Pack> packs::scan() {
	std::vector<Pack> found;
	std::error_code ec;
	auto dir = packsDir();
	fs::create_directories(dir, ec);
	auto unzipped = Mod::get()->getSaveDir() / "unzipped";

	for (auto& entry : fs::directory_iterator(dir, ec)) {
		auto path = entry.path();
		std::string name = path.stem().string();
		if (entry.is_regular_file(ec) && path.extension() == ".zip") {
			auto out = unzipped / path.stem();
			bool needs = !fs::exists(out, ec);
			if (!needs) {
				auto zt = fs::last_write_time(path, ec);
				auto ot = fs::last_write_time(out, ec);
				if (zt > ot) {
					fs::remove_all(out, ec);
					needs = true;
				}
			}
			if (needs) {
				fs::create_directories(out, ec);
				auto r = file::Unzip::intoDir(path, out);
				if (!r) {
					log::warn("Failed to unzip {}", path.string());
					continue;
				}
			}
			addPack(found, resolveRoot(out), name);
		} else if (entry.is_directory(ec)) {
			addPack(found, resolveRoot(path), path.filename().string());
		}
	}

	auto order = Mod::get()->getSavedValue<std::vector<std::string>>("pack-order");
	auto enabled = Mod::get()->getSavedValue<std::vector<std::string>>("enabled-packs");

	for (auto& p : found) {
		p.enabled = std::find(enabled.begin(), enabled.end(), p.id) != enabled.end();
	}

	auto rank = [&](Pack const& p) -> size_t {
		auto it = std::find(order.begin(), order.end(), p.id);
		return it == order.end() ? order.size() : static_cast<size_t>(it - order.begin());
	};
	std::stable_sort(found.begin(), found.end(), [&](Pack const& a, Pack const& b) {
		return rank(a) < rank(b);
	});
	return found;
}

void packs::save(std::vector<Pack> const& list) {
	std::vector<std::string> order, enabled;
	for (auto& p : list) {
		order.push_back(p.id);
		if (p.enabled) enabled.push_back(p.id);
	}
	Mod::get()->setSavedValue("pack-order", order);
	Mod::get()->setSavedValue("enabled-packs", enabled);
	(void)Mod::get()->saveData();
}

void packs::applyEnabled() {
	auto list = scan();
	auto fu = CCFileUtils::sharedFileUtils();
	std::error_code ec;
	// priority paths are added to the front, so add lowest priority first
	for (auto it = list.rbegin(); it != list.rend(); ++it) {
		if (!it->enabled) continue;
		fu->addPriorityPath(it->path.string().c_str());
		auto res = it->path / "resources";
		if (fs::exists(res, ec)) {
			fu->addPriorityPath(res.string().c_str());
		}
		log::info("Texture pack enabled: {}", it->name);
	}
}
