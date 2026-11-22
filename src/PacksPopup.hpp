#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include "Packs.hpp"

class PacksPopup : public geode::Popup {
protected:
	std::vector<Pack> m_packs;
	geode::ScrollLayer* m_scroll = nullptr;

	bool init();
	void refresh();
	void onToggle(cocos2d::CCObject* sender);
	void onMove(cocos2d::CCObject* sender);
	void onApply(cocos2d::CCObject*);
	void onOpenFolder(cocos2d::CCObject*);

public:
	static PacksPopup* create();
};
