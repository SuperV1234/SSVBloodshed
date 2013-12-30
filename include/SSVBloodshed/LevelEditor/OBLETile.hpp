// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_LEVELEDITOR_TILE
#define SSVOB_LEVELEDITOR_TILE

#include "SSVBloodshed/OBCommon.hpp"
#include "SSVBloodshed/OBAssets.hpp"
#include "SSVBloodshed/LevelEditor/OBLEDatabase.hpp"

namespace ob
{
	class OBLETile
	{
		SSVUJ_CONVERTER_FRIEND();

		private:
			int x{-1}, y{-1}, z{-1};
			OBLETType type{OBLETType::LETNull};
			std::map<std::string, ssvuj::Obj> params;

			sf::Sprite sprite;
			ssvu::Uptr<ssvs::BitmapText> idText{nullptr};

		public:
			inline OBLETile() = default;
			inline OBLETile(const OBLETile& mTile) noexcept : x{mTile.x}, y{mTile.y}, z{mTile.z}, type{mTile.type}, params{mTile.params} { }

			inline void refreshIdText(OBAssets& mAssets)
			{
				if(hasParam("id") && idText == nullptr)
				{
					idText.reset(new ssvs::BitmapText(*mAssets.obStroked));
					idText->setScale(0.5f, 0.5f);
					idText->setTracking(-3);
				}
			}

			inline void setRot(int mDeg) noexcept { if(hasParam("rot")) ssvuj::arch(params["rot"], mDeg); }
			inline void setId(OBAssets& mAssets, int mId) noexcept
			{
				if(!hasParam("id")) return;
				ssvuj::arch(params["id"], mId);
				refreshIdText(mAssets);
			}

			inline void setParam(const std::string& mKey, const std::string& mValue)
			{
				auto& p(params[mKey]);
				try
				{
					if(ssvuj::isObjType<int>(p)) p = std::stoi(mValue);
					else if(ssvuj::isObjType<float>(p)) p = std::stof(mValue);
					else if(ssvuj::isObjType<bool>(p)) p = mValue == "true" ? true : false;
				}
				catch(const std::exception& mError)
				{
					ssvu::lo("ob::OBLETile::setParam") << "Error setting parameter <" << mKey << ">: <" << mValue << ">!\n";
					ssvu::lo("ob::OBLETile::setParam") << mError.what() << std::endl;
				}
			}

			inline void update()
			{
				sprite.setOrigin(sprite.getTextureRect().width / 2.f, sprite.getTextureRect().height / 2.f);
				sprite.setRotation(params.count("rot") > 0 ? ssvuj::getExtr<int>(params["rot"]) : 0);
				sprite.setPosition(x * 10.f, y * 10.f);

				if(idText != nullptr)
				{
					auto id(getParam<int>("id"));
					idText->setPosition(x * 10.f - 4, y * 10.f - 5);
					idText->setString(id == -1 ? "" : ssvu::toStr(id));
				}
			}

			inline void initFromEntry(const OBLEDatabaseEntry& mEntry) noexcept
			{
				idText.reset(nullptr);
				type = mEntry.type; params = mEntry.defaultParams;
			}
			inline void initGfxFromEntry(const OBLEDatabaseEntry& mEntry) noexcept
			{
				sprite.setTexture(*mEntry.texture);
				sprite.setTextureRect(mEntry.intRect);
			}

			inline OBLETile& operator=(const OBLETile& mT) noexcept { x = mT.x; y = mT.y; z = mT.z; type = mT.type; params = mT.params; return *this; }

			inline bool isNull() const noexcept								{ return type == OBLETType::LETNull; }
			inline void setX(int mX) noexcept								{ x = mX; }
			inline void setY(int mY) noexcept								{ y = mY; }
			inline void setZ(int mZ) noexcept								{ z = mZ; }
			inline void setType(OBLETType mType) noexcept					{ type = mType; }
			inline void setParams(decltype(params) mParams)					{ params = std::move(mParams); }
			template<typename T> inline T getParam(const std::string& mKey)	{ return ssvuj::getExtr<T>(params[mKey]); }
			inline bool hasParam(const std::string& mKey) const noexcept	{ return params.count(mKey) > 0; }

			inline OBLETType getType() const noexcept					{ return type; }
			inline int getX() const noexcept							{ return x; }
			inline int getY() const noexcept							{ return y; }
			inline int getZ() const noexcept							{ return z; }
			inline const sf::Sprite& getSprite() const noexcept			{ return sprite; }
			inline sf::Sprite& getSprite() noexcept						{ return sprite; }
			inline const decltype(params)& getParams() const noexcept	{ return params; }
			inline decltype(params)& getParams() noexcept				{ return params; }
			inline ssvs::BitmapText* getIdText() noexcept				{ return idText.get(); }
	};
}

#endif

