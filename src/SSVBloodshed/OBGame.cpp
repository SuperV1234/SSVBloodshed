// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVBloodshed/OBGame.h"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvu::FileSystem;
using namespace ssvs;
using namespace ssvsc;
using namespace ssvsc::Utils;
using namespace sses;

namespace ob
{
	OBGame::OBGame(GameWindow& mGameWindow, OBAssets& mAssets) : gameWindow(mGameWindow), assets(mAssets), factory{assets, *this, manager, world},
		world(createResolver<Impulse>(), createSpatial<HashGrid>(1000, 1000, 3000, 500)),  debugText{assets.get<BitmapFont>("limeStroked")}
	{
		gameState.onUpdate += [this](float mFrameTime){ update(mFrameTime); };
		gameState.onDraw += [this]{ draw(); };

		// Debug
		debugText.setTracking(-3);

		// Input
		using k = Keyboard::Key;
		using b = Mouse::Button;
		using t = Input::Trigger::Type;

		gameState.addInput({{k::Escape}}, [this](float){ assets.musicPlayer.stop(); std::terminate(); });

		gameState.addInput({{k::A}}, [=](float){ camera.pan(-10, 0); });
		gameState.addInput({{k::D}}, [=](float){ camera.pan(10, 0); });
		gameState.addInput({{k::W}}, [=](float){ camera.pan(0, -10); });
		gameState.addInput({{k::S}}, [=](float){ camera.pan(0, 10); });
		gameState.addInput({{k::Q}}, [=](float){ camera.zoomOut(1.1f); });
		gameState.addInput({{k::E}}, [=](float){ camera.zoomIn(1.1f); });

		add2StateInput(gameState, {{k::Z}}, inputShoot);
		add2StateInput(gameState, {{k::X}}, inputSwitch, t::Once);

		add3StateInput(gameState, {{k::Left}}, {{k::Right}}, inputX);
		add3StateInput(gameState, {{k::Up}}, {{k::Down}}, inputY);

		gameState.addInput({{k::R}}, [this](float){ newGame(); }, t::Once);

		newGame();
	}

	void OBGame::newGame()
	{
		manager.clear();

		auto getTilePos = [](int mX, int mY) -> Vec2i { return toCoords(Vec2i{mX * 10 + 5, mY * 10 + 5}); };
		constexpr int maxX{320 / 10}, maxY{240 / 10};

		for(int iX{0}; iX < maxX; ++iX)
			for(int iY{0}; iY < maxY; ++iY)
			{
				if(iX == 0 || iX == maxX - 1 || iY == 0 || iY == maxY - 1) factory.createWall(getTilePos(iX, iY));
				else factory.createFloor(getTilePos(iX, iY));
			}
	}

	void OBGame::update(float mFrameTime)
	{
		manager.update(mFrameTime);
		world.update(mFrameTime);
		updateDebugText(mFrameTime);
		camera.update(mFrameTime);
	}
	void OBGame::updateDebugText(float mFrameTime)
	{
		ostringstream s;
		const auto& entities(manager.getEntities());
		const auto& bodies(world.getBodies());
		std::size_t componentCount{0}, dynamicBodiesCount{0};
		for(const auto& e : entities) componentCount += e->getComponents().size();
		for(const auto& b : bodies) if(!b->isStatic()) ++dynamicBodiesCount;

		s << "FPS: "				<< gameWindow.getFPS() << endl;
		s << "FrameTime: "			<< mFrameTime << endl;
		s << "Bodies(all): "		<< bodies.size() << endl;
		s << "Bodies(static): "		<< bodies.size() - dynamicBodiesCount << endl;
		s << "Bodies(dynamic): "	<< dynamicBodiesCount << endl;
		s << "Sensors: "			<< world.getSensors().size() << endl;
		s << "Entities: "			<< entities.size() << endl;
		s << "Components: "			<< componentCount << endl;

		debugText.setString(s.str());
	}

	void OBGame::draw()
	{
		camera.apply<int>();
		manager.draw();
		camera.unapply();
		render(debugText);
	}
}
