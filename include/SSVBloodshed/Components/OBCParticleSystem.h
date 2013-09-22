// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_COMPONENTS_PARTICLESYSTEM
#define SSVOB_COMPONENTS_PARTICLESYSTEM

#include "SSVBloodshed/OBCommon.h"
#include "SSVBloodshed/OBGame.h"
#include "SSVBloodshed/Components/OBCPhys.h"
#include "SSVBloodshed/Components/OBCDraw.h"

namespace ob
{
	class Particle
	{
		private:
			Vec2f position, velocity;
			float acceleration{0.f};
			sf::Color color;
			float size{1.f}, life{100.f}, lifeMax{100.f}, alphaMult{1.f};

		public:
			inline Particle(const Vec2f& mPosition, const Vec2f& mVelocity, float mAcceleration, const sf::Color& mColor, float mSize, float mLife, float mAlphaMult)
				: position{mPosition}, velocity{mVelocity}, acceleration{mAcceleration}, color{mColor}, size{mSize}, life{mLife}, lifeMax{mLife}, alphaMult{mAlphaMult} { }

			inline void update(float mFrameTime)
			{
				life -= mFrameTime;
				color.a = static_cast<unsigned char>(ssvu::getClamped(life * (255.f / lifeMax) * alphaMult, 0.f, 255.f));
				velocity *= acceleration; // TODO: * mFrameTime?
				position += velocity * mFrameTime;
			}

			inline void setPosition(const Vec2f& mPosition) noexcept	{ position = mPosition; }
			inline void setColor(const sf::Color& mColor) noexcept		{ color = mColor; }
			inline void setVelocity(const Vec2f& mVelocity) noexcept	{ velocity = mVelocity; }
			inline void setAcceleration(float mAcceleration) noexcept	{ acceleration = mAcceleration; }
			inline void setLife(float mLife) noexcept					{ life = lifeMax = mLife; }
			inline void setOpacityMult(float mAlphaMult) noexcept		{ alphaMult = mAlphaMult; }
			inline void setSize(float mSize) noexcept					{ size = mSize; }

			inline float getLife() const noexcept						{ return life; }
			inline float getSize() const noexcept						{ return size; }
			inline const Vec2f& getPosition() const noexcept			{ return position; }
			inline const sf::Color& getColor() const noexcept			{ return color; }
	};

	class ParticleSystem : public sf::Drawable
	{
		private:
			unsigned int maxParticles{10000};
			sf::VertexArray vertices{sf::PrimitiveType::Quads};
			std::vector<Particle> particles;

		public:
			inline ParticleSystem() { particles.reserve(maxParticles); }
			inline void emplace(const Vec2f& mPosition, const Vec2f& mVelocity, float mAcceleration, const sf::Color& mColor, float mSize, float mLife, float mAlphaMult = 1.f)
			{
				if(particles.size() >= maxParticles) return;
				particles.emplace_back(mPosition, mVelocity, mAcceleration, mColor, mSize, mLife, mAlphaMult);
			}
			inline void update(float mFrameTime)
			{
				ssvu::eraseRemoveIf(particles, [](const Particle& mParticle){ return mParticle.getLife() <= 0; });
				vertices.clear();
				for(auto& p : particles)
				{
					p.update(mFrameTime);
					vertices.append({{p.getPosition().x - p.getSize(), p.getPosition().y - p.getSize()}, p.getColor()});
					vertices.append({{p.getPosition().x + p.getSize(), p.getPosition().y - p.getSize()}, p.getColor()});
					vertices.append({{p.getPosition().x + p.getSize(), p.getPosition().y + p.getSize()}, p.getColor()});
					vertices.append({{p.getPosition().x - p.getSize(), p.getPosition().y + p.getSize()}, p.getColor()});
				}
			}
			inline void draw(sf::RenderTarget& mRenderTarget, sf::RenderStates mRenderStates) const override { mRenderTarget.draw(vertices, mRenderStates); }
			inline void clear() { particles.clear(); }
	};

	class OBCParticleSystem : public sses::Component
	{
		private:
			sf::RenderTexture& renderTexture;
			sf::RenderTarget& renderTarget;
			bool clearOnDraw{false};
			unsigned char alpha{255};
			ParticleSystem particleSystem;
			sf::Sprite sprite;

		public:
			OBCParticleSystem(sf::RenderTexture& mRenderTexture, sf::RenderTarget& mRenderTarget, bool mClearOnDraw = false, unsigned char mAlpha = 255)
				: renderTexture(mRenderTexture), renderTarget(mRenderTarget), clearOnDraw{mClearOnDraw}, alpha{mAlpha} { }

			inline void init() override
			{
				renderTexture.clear(sf::Color::Transparent);
				sprite.setTexture(renderTexture.getTexture());
				sprite.setColor({255, 255, 255, alpha});
			}
			inline void update(float mFrameTime) override { particleSystem.update(mFrameTime); }
			inline void draw() override
			{
				renderTexture.draw(particleSystem);
				renderTexture.display();
				renderTarget.draw(sprite);
				if(clearOnDraw) renderTexture.clear(sf::Color::Transparent);
			}

			inline ParticleSystem& getParticleSystem() noexcept				{ return particleSystem; }
			inline const ParticleSystem& getParticleSystem() const noexcept	{ return particleSystem; }
	};

	inline void createPBlood(ParticleSystem& mPS, const Vec2f& mPosition, float mMult)
	{
		mPS.emplace(mPosition,
				ssvs::getVecFromDegrees(ssvu::getRndR<float>(0.f, 360.f), ssvu::getRndR<float>(0.f, 13.f * mMult)),
				0.9f,
				{ssvu::getRnd<unsigned char>(185, 255), 0, 0, 255},
				1.f + ssvu::getRndR<float>(-0.3, 0.3),
				75 + ssvu::getRnd(-65, 65),
				0.42f);
	}
	inline void createPGib(ParticleSystem& mPS, const Vec2f& mPosition)
	{
		mPS.emplace(mPosition,
				ssvs::getVecFromDegrees(ssvu::getRndR<float>(0.f, 360.f), ssvu::getRndR<float>(0.1f, 13.f)),
				0.93f,
				{ssvu::getRnd<unsigned char>(95, 170), 15, 15, 255},
				1.1f + ssvu::getRndR<float>(-0.3, 0.3),
				150 + ssvu::getRnd(-50, 50),
				1.5f);
	}
	inline void createPDebris(ParticleSystem& mPS, const Vec2f& mPosition)
	{
		mPS.emplace(mPosition,
				ssvs::getVecFromDegrees(ssvu::getRndR<float>(0.f, 360.f), ssvu::getRndR<float>(1.f, 9.f)),
				0.9f,
				sf::Color::Black,
				1.f + ssvu::getRndR<float>(-0.3, 0.3),
				65 + ssvu::getRnd(-50, 50),
				0.8f);
	}
	inline void createPDebrisFloor(ParticleSystem& mPS, const Vec2f& mPosition)
	{
		mPS.emplace(mPosition,
				ssvs::getVecFromDegrees(ssvu::getRndR<float>(0.f, 360.f), ssvu::getRndR<float>(0.5f, 3.4f)),
				0.9f,
				{179, 179, 179, 255},
				2.5f,
				65 + ssvu::getRnd(-50, 50),
				0.8f);
	}
	inline void createPMuzzle(ParticleSystem& mPS, const Vec2f& mPosition)
	{
		mPS.emplace(mPosition,
				ssvs::getVecFromDegrees(ssvu::getRndR<float>(0.f, 360.f), ssvu::getRndR<float>(1.f, 4.5f)),
				0.9f,
				{255, ssvu::getRnd<unsigned char>(95, 100), 15, 255},
				1.1f + ssvu::getRndR<float>(-0.3, 0.3),
				6 + ssvu::getRnd(-5, 5),
				1.5f);
	}
}

#endif
