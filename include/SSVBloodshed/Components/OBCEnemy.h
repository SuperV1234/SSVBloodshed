// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_COMPONENTS_ENEMY
#define SSVOB_COMPONENTS_ENEMY

#include "SSVBloodshed/OBCommon.h"
#include "SSVBloodshed/OBGame.h"
#include "SSVBloodshed/Components/OBCPhysics.h"
#include "SSVBloodshed/Components/OBCRender.h"

namespace ob
{
	class OBCEnemy : public sses::Component
	{
		public:
			OBGame& game;
			OBCPhysics& cPhysics;
			OBCRender& cRender;
			OBAssets& assets;
			ssvsc::Body& body;
			float walkSpeed{100.f}, currentDegrees{0.f}, turnSpeed{7.5f};
			float snappedDegrees{0.f};
			int health{3}; int gibMult{1};
			//ssvs::Ticker shootTimer{4.7f};

		public:
			OBCEnemy(OBGame& mGame, OBCPhysics& mCPhysics, OBCRender& mCRender, OBAssets& mAssets) : game(mGame), cPhysics(mCPhysics), cRender(mCRender), assets(mAssets), body(cPhysics.getBody()) { }

			inline void init() override
			{
				body.setRestitutionX(1.7f);
				body.setRestitutionY(1.7f);
				body.onPreUpdate += [this]{ body.setVelocity(ssvs::getMClamped(body.getVelocity(), -120.f, 120.f)); };
				body.onDetection += [this](const ssvsc::DetectionInfo& mDI)
				{
					if(mDI.body.hasGroup(OBGroup::Projectile))
					{
						--health;
						for(int i{0}; i < 8; ++i) game.getPSPerm().createBlood(toPixels(body.getPosition()));

						if(health <= 0)
						{
							for(int i{0}; i < 25 * gibMult; ++i) game.getPSPerm().createBlood(toPixels(body.getPosition()));
							for(int i{0}; i < 60 * gibMult; ++i) game.getPSTemp().createGib(toPixels(body.getPosition()));
							getEntity().destroy();
						}
					}
				};
			}
			inline void update(float mFrameTime) override
			{
				for(const auto& e : game.getManager().getEntities(OBGroup::Player))
				{
					auto& ecPhysics(e->getComponent<OBCPhysics>());
					float targetDegrees(ssvs::getDegreesTowards(Vec2f(body.getPosition()), Vec2f(ecPhysics.getBody().getPosition())));

					currentDegrees = ssvu::getRotatedDegrees(currentDegrees, targetDegrees, turnSpeed * mFrameTime);
					snappedDegrees = static_cast<int>(currentDegrees / 45.f) * 45.f;

					body.applyForce(ssvs::getVecFromDegrees(snappedDegrees, walkSpeed) * 0.05f);
				}
			}
			inline void draw() override
			{
				auto& s0(cRender[0]);
				//s0.setTextureRect(assets.get<Tileset>("tileset")Enemy[{0, 0}]);
				s0.setRotation(snappedDegrees);
			}
	};
}

#endif