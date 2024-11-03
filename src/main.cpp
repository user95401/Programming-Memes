#include <_main.hpp>
#include <Geode/utils/web.hpp>

//popup logic and on game loaded showup
#include <Geode/modify/CCScene.hpp>
class $modify(PopupRandomMeme, CCScene) {

    //originally it was called "can_asd"
    inline static bool can_be_showed_on_next_scene_after_loading_layer = 0;

    inline static Ref<CCArray> layers_history = CCArray::create();
    inline static std::set<std::string> images_history;

    static void loadAndShow() {

        web::WebRequest req = web::WebRequest();
        req.header(
            "x-rapidapi-key",//its not my key, i take it from demo. :>
            ZipUtils::base64URLDecode("MWI0NzE0YTFiNW1zaDY0YmM3NDdmODg3YjcyOHAxNTViMjNqc25jN2UyN2I2N2ExOTM=")
        );

        auto listener = new EventListener<web::WebTask>;
        listener->bind(
            [listener](web::WebTask::Event* e) {
                if (web::WebResponse* res = e->getValue()) {

                    //log::info("headers: {}", res->headers());
                    if (string::contains(res->header("content-type").value_or("asd"), "image")) {
                        //log::info("image");
                        //don't specify exact image ext if you not sure for real format
                        //game can't load jpg as .png, it will try load it as png and get failed.
                        auto path = getMod()->getTempDir() / ".rand_meme_image";
                        res->into(path);
                        auto sprite = CCSprite::create(path.string().c_str());
                        //sprite has cached frame so temp file can be deleted.
#ifndef __APPLE__
                        CCTextureCache::get()->removeTextureForKey(path.string().c_str());
#endif
                        fs::remove(path, fs::last_err_code);

                        auto noElasticity = false;
                        if (auto MemePopup = typeinfo_cast<FLAlertLayer*>(
                            CCScene::get()->getChildByIDRecursive("MemePopup")
                        )) {
                            MemePopup->onBtn1(MemePopup);
                            noElasticity = 1;
                        }

                        auto popup = createQuickPopup("", "", "", "", [](auto, auto) {}, 0);
                        popup->setID("MemePopup");
                        popup->m_noElasticity = noElasticity;
                        popup->show();

                        //clear childs but keep alive and add menu
                        popup->m_mainLayer->removeAllChildrenWithCleanup(0);
                        popup->m_mainLayer->addChild(popup->m_buttonMenu);
                        
                        //close btn
                        popup->m_button1->getParent()->setContentSize(CCSizeMake(1, 1) * 7777);
                        popup->m_button1->setVisible(0);

                        popup->m_button2->setVisible(0);

                        auto layer = CCLayer::create();
                        layer->addChild(sprite);
                        sprite->setPosition(layer->getContentSize() / 2);
                        limitNodeSize(sprite, layer->getContentSize() - CCSize(60, 40), 8.f, 0.1f);

                        layers_history->insertObject(layer, 0);

                        auto boomscroll = BoomScrollLayer::create(layers_history, 0, 0);
                        auto indicators = getChildOfType<CCSpriteBatchNode>(boomscroll, 0);
                        indicators->setPositionY(6.f);
                        indicators->setContentWidth(boomscroll->getContentWidth());
                        indicators->setLayout(RowLayout::create());
                        indicators->addChild(LambdaNode::createToEndlessCalls(
                            [indicators]() {
                                if (indicators) indicators->updateLayout();
                            }
                        ));

                        popup->m_mainLayer->addChild(boomscroll);

                        return;
                    }

                    auto json = res->json();
                    auto value = matjson::Value(json.value_or(json.error_or("unk err")));
                    //log::info("{}", value);

                    if (json.has_error()) return;
                    if (value.is_array() == false) return;
                    //log::info("{}", "as_array");
                    //log::info("value {}", value);
                    auto the_memes_list = value.as_array();
                    //get random meme from list and skip used ones
                    get_rand:
                    matjson::Value rand_meme = *select_randomly(the_memes_list.begin(), the_memes_list.end());
                    auto image = rand_meme.try_get<std::string>("image").value_or("no meme?(");
                    while (images_history.contains(image)) {
                        //log::warn("skippnig meme. {}", image);
                        goto get_rand;
                    }
                    images_history.insert(image);
                    //log::info("rand_meme {}", rand_meme);
                    web::WebRequest req = web::WebRequest();
                    listener->setFilter(req.get(image));
                }
            }
        );
        listener->setFilter(req.get("https://programming-memes-images.p.rapidapi.com/v1/memes"));
    };

    void asd(float) {
        if (this->getChildByIDRecursive("LoadingLayer")) {
            //wait for next scene
            can_be_showed_on_next_scene_after_loading_layer = true;
            return;
        }

        //dont show if not wait
        if (!can_be_showed_on_next_scene_after_loading_layer) return;

        //show and mark already showed
        loadAndShow();
        can_be_showed_on_next_scene_after_loading_layer = false;
    };

    $override static CCScene* create() {
        auto __this = CCScene::create();
        if (SETTING(bool, "Show After Loading")) {
            __this->scheduleOnce(schedule_selector(PopupRandomMeme::asd), 0.0f);
        };
        return __this;
    }

};

//define same type name idk? im not learned any of rtti stuff)
class ModsLayer : CCNode {};

#include <Geode/modify/CCLayer.hpp>
class $modify(ModsLayerExt, CCLayer) {
    $override bool init() {
        if (!CCLayer::init()) return false;

        if (typeinfo_cast<ModsLayer*>(this)) queueInMainThread(
            [this] {
                auto label = CCLabelBMFont::create("get random programming meme", "gjFont41.fnt");
                label->setID("label"_spr);
                label->setScale(0.3f);
                label->setBlendFunc({ GL_ONE, GL_ONE });
                auto menu = CCMenu::createWithItem(CCMenuItemExt::createSpriteExtra(
                    label, [](auto) {
                        //PopupRandomMeme::loadAndShow();
                        
                        //test
                        auto openup = typeinfo_cast<CCMenuItem*>(GameManager::get()->getChildByID("popup-random-meme"));
                        if (openup) openup->activate();
                    }
                ));
                menu->addChild(LambdaNode::createToEndlessCalls(
                    [menu]() {
                        if (menu) menu->setPositionX(SETTING(double, "Button Position X"));
                        if (menu) menu->setPositionY(SETTING(double, "Button Position Y"));
                    }
                ));
                menu->setID("menu"_spr);
                this->addChild(menu);
            }
        );

        return true;
}
};

inline void on_mod_loaded() {
    if (auto manager = GameManager::get()) {
        auto openup = CCMenuItemExt::create(
            [](auto) {
                PopupRandomMeme::loadAndShow();
            }
        );
        openup->setID("popup-random-meme");
        manager->addChild(openup);
    }
}

$on_mod(Loaded) {
    //intelsense works not fully in macro hell
    on_mod_loaded();
}