#include "PacksPopup.hpp"

using namespace geode::prelude;

PacksPopup* PacksPopup::create() {
	auto ret = new PacksPopup();
	if (ret->initAnchored(340.f, 240.f)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool PacksPopup::setup() {
	this->setTitle("Texture Packs");
	m_packs = packs::scan();

	auto size = m_mainLayer->getContentSize();
	float listW = 300.f, listH = 140.f;
	float cy = 130.f;

	auto bg = CCScale9Sprite::create("square02b_001.png", {0, 0, 80, 80});
	bg->setColor({0, 0, 0});
	bg->setOpacity(90);
	bg->setContentSize({listW, listH});
	bg->setPosition({size.width / 2, cy});
	m_mainLayer->addChild(bg);

	m_scroll = ScrollLayer::create({listW, listH});
	m_scroll->setPosition({(size.width - listW) / 2, cy - listH / 2});
	m_mainLayer->addChild(m_scroll);

	auto menu = CCMenu::create();
	menu->setPosition({0, 0});
	m_mainLayer->addChild(menu);

	auto apply = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Apply"), this, menu_selector(PacksPopup::onApply)
	);
#if defined(GEODE_IS_DESKTOP)
	apply->setPosition({size.width / 2 + 50.f, 30.f});
	auto folder = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Open Folder"), this, menu_selector(PacksPopup::onOpenFolder)
	);
	folder->setPosition({size.width / 2 - 70.f, 30.f});
	menu->addChild(folder);
#else
	apply->setPosition({size.width / 2, 30.f});
#endif
	menu->addChild(apply);

	this->refresh();
	return true;
}

void PacksPopup::refresh() {
	auto content = m_scroll->m_contentLayer;
	content->removeAllChildren();

	float w = 300.f, rowH = 34.f;
	float total = std::max(m_scroll->getContentSize().height, m_packs.size() * rowH);
	content->setContentSize({w, total});

	if (m_packs.empty()) {
		auto text = fmt::format("No packs found.\nPut packs in:\n{}", packs::packsDir().string());
		auto label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
		label->setScale(0.6f);
		label->setPosition({w / 2, total / 2});
		content->addChild(label);
		return;
	}

	for (size_t i = 0; i < m_packs.size(); i++) {
		auto& pack = m_packs[i];
		float y = total - (i + 0.5f) * rowH;

		auto menu = CCMenu::create();
		menu->setPosition({0, y});
		content->addChild(menu);

		auto toggler = CCMenuItemToggler::createWithStandardSprites(
			this, menu_selector(PacksPopup::onToggle), 0.6f
		);
		toggler->setTag(static_cast<int>(i));
		toggler->toggle(pack.enabled);
		toggler->setPosition({22.f, 0.f});
		menu->addChild(toggler);

		auto upSpr = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
		upSpr->setScale(0.7f);
		auto up = CCMenuItemSpriteExtra::create(upSpr, this, menu_selector(PacksPopup::onMove));
		up->setTag(static_cast<int>(i));
		up->setPosition({245.f, 0.f});
		menu->addChild(up);

		auto downSpr = CCSprite::createWithSpriteFrameName("edit_downBtn_001.png");
		downSpr->setScale(0.7f);
		auto down = CCMenuItemSpriteExtra::create(downSpr, this, menu_selector(PacksPopup::onMove));
		down->setTag(static_cast<int>(i) + 10000);
		down->setPosition({275.f, 0.f});
		menu->addChild(down);

		auto label = CCLabelBMFont::create(pack.name.c_str(), "bigFont.fnt");
		label->setAnchorPoint({0.f, 0.5f});
		label->limitLabelWidth(160.f, 0.5f, 0.1f);
		label->setPosition({45.f, y});
		content->addChild(label);
	}
	m_scroll->moveToTop();
}

void PacksPopup::onToggle(CCObject* sender) {
	auto toggler = static_cast<CCMenuItemToggler*>(sender);
	int i = toggler->getTag();
	if (i >= 0 && i < static_cast<int>(m_packs.size())) {
		// callback fires before the toggler flips its state
		m_packs[i].enabled = !toggler->isToggled();
	}
}

void PacksPopup::onMove(CCObject* sender) {
	int tag = static_cast<CCNode*>(sender)->getTag();
	bool down = tag >= 10000;
	int i = down ? tag - 10000 : tag;
	int j = down ? i + 1 : i - 1;
	if (i < 0 || j < 0 || i >= static_cast<int>(m_packs.size()) || j >= static_cast<int>(m_packs.size())) return;
	std::swap(m_packs[i], m_packs[j]);
	this->refresh();
}

void PacksPopup::onApply(CCObject*) {
	packs::save(m_packs);
#if defined(GEODE_IS_DESKTOP)
	createQuickPopup(
		"Restart", "Texture packs are applied on startup.\nRestart the game now?",
		"Later", "Restart",
		[](auto, bool restart) {
			if (restart) game::restart();
		}
	);
#else
	FLAlertLayer::create(
		"Saved", "Texture packs are applied on startup.\nFully close and reopen the game.", "OK"
	)->show();
#endif
}

void PacksPopup::onOpenFolder(CCObject*) {
#if defined(GEODE_IS_DESKTOP)
	file::openFolder(packs::packsDir());
#endif
}
