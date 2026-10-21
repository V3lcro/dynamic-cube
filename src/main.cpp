#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(DynamicCubePlayer, PlayerObject) {
    struct Fields {
        bool pending = false;
        int framesLeft = 0;
        bool releasing = false;
    };

    static bool isCubeMode(PlayerObject* p) {
        return !p->m_isShip && !p->m_isBall && !p->m_isBird
            && !p->m_isDart && !p->m_isRobot && !p->m_isSpider
            && !p->m_isSwing;
    }

    void pushButton(PlayerButton button) {
        if (button != PlayerButton::Jump) {
            PlayerObject::pushButton(button);
            return;
        }

        auto* mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled") || !isCubeMode(this)) {
            PlayerObject::pushButton(button);
            return;
        }

        // Simulated re-press from update() must pass through.
        if (m_fields->releasing) {
            PlayerObject::pushButton(button);
            return;
        }

        // Already grounded: normal jump, no reason to delay.
        if (m_isOnGround2) {
            PlayerObject::pushButton(button);
            return;
        }

        // Airborne press: hold it and release on the first grounded tick.
        m_fields->pending = true;
        m_fields->framesLeft =
            static_cast<int>(mod->getSettingValue<int64_t>("early-window"));
    }

    void releaseButton(PlayerButton button) {
        // Swallow the physical release while the logical press is buffered.
        if (button == PlayerButton::Jump && m_fields->pending) {
            m_fields->pending = false;
            return;
        }
        PlayerObject::releaseButton(button);
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!m_fields->pending) return;

        if (m_fields->framesLeft <= 0) {
            m_fields->pending = false;
            return;
        }

        m_fields->framesLeft--;

        if (m_isOnGround2) {
            m_fields->pending = false;
            m_fields->releasing = true;
            PlayerObject::pushButton(PlayerButton::Jump);
            PlayerObject::releaseButton(PlayerButton::Jump);
            m_fields->releasing = false;
        }
    }
};