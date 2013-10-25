// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_COMPONENTS_TARGETER
#define SSVOB_COMPONENTS_TARGETER

#include "SSVBloodshed/OBCommon.h"
#include "SSVBloodshed/OBGame.h"
#include "SSVBloodshed/Components/OBCActorBase.h"

namespace ob
{
	class OBCTargeter : public OBCActorNoDrawBase
	{
		private:
			OBCPhys* target{nullptr};
			OBGroup targetGroup;
			sses::EntityStat targetStat;
			float distance;

		public:
			OBCTargeter(OBCPhys& mCPhys, OBGroup mTargetGroup) noexcept : OBCActorNoDrawBase{mCPhys}, targetGroup(mTargetGroup) { }

			inline void update(float) override
			{
				if(target == nullptr && manager.hasEntity(targetGroup))
				{
					const auto& e(manager.getEntities(targetGroup).front());
					targetStat = e->getStat(); target = &e->getComponent<OBCPhys>();
					distance = ssvs::getDistEuclidean(target->getPosF(), cPhys.getPosF());
				}
				else if(!manager.isAlive(targetStat)) target = nullptr;
			}

			inline bool hasTarget() const noexcept			{ return target != nullptr && manager.isAlive(targetStat); }
			inline OBCPhys& getTarget() const noexcept		{ return *target; }
			inline Vec2f getPosF() const noexcept			{ return target->getPosF(); }
			inline const Vec2i& getPosI() const noexcept	{ return target->getPosI(); }
			inline const Vec2f& getVel() const noexcept		{ return target->getVel(); }
			inline float getDist() const noexcept			{ return distance; }
	};
}

#endif

