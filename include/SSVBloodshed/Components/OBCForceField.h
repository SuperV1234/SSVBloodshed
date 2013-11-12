// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_COMPONENTS_FORCEFIELD
#define SSVOB_COMPONENTS_FORCEFIELD

#include "SSVBloodshed/OBCommon.h"
#include "SSVBloodshed/OBGame.h"
#include "SSVBloodshed/Components/OBCActorBase.h"
#include "SSVBloodshed/Components/OBCIdReceiver.h"

namespace ob
{
	class OBCForceField : public OBCActorBase
	{
		private:
			OBCIdReceiver& cIdReceiver;
			Dir8 dir;
			ssvsc::Segment<float> segment;
			bool active{true}, destroyProjectiles, blockFriendly, blockEnemy, booster{false};
			float distortion{0}, alpha{0}, forceMult;
			ssvs::Animation animation;

		public:
			OBCForceField(OBCPhys& mCPhys, OBCDraw& mCDraw, OBCIdReceiver& mCIdReceiver, Dir8 mDir, bool mDestroyProjectiles, bool mBlockFriendly, bool mBlockEnemy, float mForceMult) noexcept
				: OBCActorBase{mCPhys, mCDraw}, cIdReceiver(mCIdReceiver), dir{mDir}, destroyProjectiles{mDestroyProjectiles}, blockFriendly{mBlockFriendly}, blockEnemy{mBlockEnemy},
				  forceMult{mForceMult}, animation{assets.aForceField} { }

			inline void init()
			{
				booster = !destroyProjectiles && !blockFriendly && !blockEnemy;

				cIdReceiver.onActivate += [this](IdAction mIdAction)
				{
					switch(mIdAction)
					{
						case IdAction::Toggle: active = !active; break;
						case IdAction::Open: active = true; break;
						case IdAction::Close: active = false; break;
					}
				};

				cDraw.setRotation(getDegFromDir8(dir));
				getEntity().addGroups(OBGroup::GForceField);
				body.setResolve(false);
				body.addGroups(OBGroup::GForceField);
				body.addGroupsToCheck(OBGroup::GProjectile, OBGroup::GFriendly, OBGroup::GEnemy);

				// Calculate the segment by orbiting the center point
				segment = {ssvs::getOrbitDeg(body.getPosition(), getDeg() + 90.f, 1500.f), ssvs::getOrbitDeg(body.getPosition(), getDeg() - 90.f, 1500.f)};

				body.onDetection += [this](const DetectionInfo& mDI)
				{
					if(!active) return;

					const auto& dirVec(-getVecFromDir8<float>(dir));

					// When something touches the force field, spawn particles
					game.createPForceField(1, toPixels(mDI.body.getPosition()));

					if(!booster)
					{
						distortion = 10;

						// If this force field blocks friendlies or enemies and the detected body matches...
						if((blockFriendly && mDI.body.hasGroup(OBGroup::GFriendly)) || (blockEnemy && mDI.body.hasGroup(OBGroup::GEnemy)))
						{
							bool isMoving{mDI.body.getVelocity().x != 0 || mDI.body.getVelocity().y != 0};

							// Check if the body is "inside" the force field (check if it's on the right side of the segment)
							if(!segment.isPointLeft(Vec2f(mDI.body.getPosition())))
							{
								// If it's not inside, push it away from the force field
								mDI.body.applyForce(dirVec * 5.f * forceMult);

								// If it's moving and it's not inside, treat the collision as a solid one
								if(isMoving && isDegBlocked(ssvs::getDeg(mDI.body.getVelocity()))) mDI.body.resolvePosition(ssvsc::Utils::getMin1DIntersection(mDI.body.getShape(), body.getShape()));
							}
							else if(!isMoving) mDI.body.applyForce(dirVec * -5.f * forceMult);
						}

						// Eventually destroy projectiles that move against the force field
						if(destroyProjectiles && mDI.body.hasGroup(OBGroup::GProjectile))
						{
							auto& cProjectile(getComponentFromBody<OBCProjectile>(mDI.body));
							if(isDegBlocked(cProjectile.getDeg())) cProjectile.destroy();
						}
					}
					else if(mDI.body.hasGroup(OBGroup::GProjectile))
					{
						if(forceMult > 0.f)
						{
							mDI.body.applyForce(dirVec * 30.f * forceMult);
						}
						else if(ssvs::getDeg(mDI.body.getVelocity()) != ssvs::getDeg(dirVec))
						{
							mDI.body.setVelocity(dirVec * ssvs::getMag(mDI.body.getVelocity()));
							mDI.body.setPosition(body.getPosition());
						}
					}
				};
			}

			inline void update(float mFT) override
			{
				auto color(cDraw[0].getColor());

				if(!active) color.a = 100;
				else
				{
					animation.update(mFT);

					if(!booster)
					{
						cDraw[0].setTextureRect((*assets.tsSmall)(animation.getTileIndex()));

						if(distortion > 0.f)
						{
							distortion -= mFT;
							cDraw.setGlobalScale(distortion <= 0.f ? 1.f : ssvu::getRndR(0.9f, 1.1f));
						}
					}

					alpha = std::fmod(alpha + mFT * 0.06f, ssvu::pi);
					color.a = 255 - std::sin(alpha) * 125;
				}

				cDraw[0].setColor(color);
			}

			inline bool isDegBlocked(float mDeg) const noexcept { return ssvu::getDistDeg(mDeg, getDeg()) <= 90.f; }

			inline Dir8 getDir() const noexcept		{ return dir; }
			inline float getDeg() const noexcept	{ return getDegFromDir8(dir); }

			inline bool destroysProjectiles() const noexcept { return destroyProjectiles; }
	};
}

#endif

