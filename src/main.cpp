#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "Packs.hpp"
#include "PacksPopup.hpp"

using namespace geode::prelude;

// Add enabled packs to the search paths before the game loads its textures
$on_mod(Loaded) {
	packs::applyEnabled();
}

class $modify(TLMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		auto spr = CircleButtonSprite::createWithSpriteFrameName(
			"GJ_paintBtn_001.png", 1.f, CircleBaseColor::Green, CircleBaseSize::Medium
		);
		auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(TLMenuLayer::onPacks));
		btn->setID("packs-button"_spr);

		if (auto menu = this->getChildByID("bottom-menu")) {
			menu->addChild(btn);
			menu->updateLayout();
		}
		return true;
	}

	void onPacks(CCObject*) {
		if (auto popup = PacksPopup::create()) popup->show();
	}
};
