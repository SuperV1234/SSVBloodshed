// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_GUI_WIDGET_INL
#define SSVOB_GUI_WIDGET_INL

#include "SSVBloodshed/OBCommon.hpp"
#include "SSVBloodshed/GUI/Widget.hpp"

namespace ob
{
	namespace GUI
	{
		template<typename T, typename... TArgs> inline T& Widget::create(TArgs&&... mArgs)
		{
			auto& result(context.allocateWidget<T>(std::forward<TArgs>(mArgs)...));
			result.setParent(*this); return result;
		}

		inline void Widget::updateRecursive(float mFT)
		{
			update(mFT);

			// Recalculate sizing
			recalculateSize(scalingX, &Widget::setWidth, &Widget::getLeft, &Widget::getRight);
			recalculateSize(scalingY, &Widget::setHeight, &Widget::getTop, &Widget::getBottom);
			recalculateFitToChildren(scalingX, scalingY);

			// TODO: Recalculate temp sizing (instant resize)
			recalculateSize(nextTempScaling, &Widget::setWidth, &Widget::getLeft, &Widget::getRight);
			recalculateSize(nextTempScaling, &Widget::setHeight, &Widget::getTop, &Widget::getBottom);
			recalculateFitToChildren(nextTempScaling, nextTempScaling);
			//if(!dirty) nextTempScaling = Scaling::Manual;

			recalculatePosition();

			if(isPressed()) context.busy = true;

			for(auto& w : children) w->updateRecursive(mFT);

			recalculateChildBounds();
			recalculateView();

			onPostUpdate();
		}
		inline void Widget::recalculateView()
		{
			viewBoundsMin = getVertexNW();
			viewBoundsMax = getVertexSE();

			const auto& rtSize(context.renderTexture.getSize());
			const auto& vbSize(viewBoundsMax - viewBoundsMin);

			float left{viewBoundsMin.x / rtSize.x};
			float top{viewBoundsMin.y / rtSize.y};
			float width{vbSize.x / rtSize.x};
			float height{vbSize.y / rtSize.y};

			view.setViewport({left, top, width, height});
			view.setSize(vbSize); view.setCenter(viewBoundsMin + vbSize / 2.f);
		}

		inline void Widget::gainExclusiveFocus()					{ context.unFocusAll(); recurseChildrenIf([this](Widget& mW){ return mW.depth == depth; }, [](Widget& mW){ mW.setFocused(true); }); }
		inline void Widget::render(const sf::Drawable& mDrawable)	{ context.render(parent != nullptr ? &parent->view : nullptr, mDrawable); }
		inline void Widget::destroyRecursive()						{ recurseChildren([this](Widget& mW){ context.del(mW); }); }
		inline void Widget::checkMouse()
		{
			if(!isVisible()) return;
			hovered = isOverlapping(getMousePos(), 2.f); if(hovered) context.hovered = true;
			pressedOld = pressed; pressed = isMBtnLeftDown() && hovered;
		}
		inline const Vec2f& Widget::getMousePos() const noexcept	{ return context.mousePos; }
		inline const Vec2f& Widget::getMousePosOld() const noexcept	{ return context.mousePosOld; }
		inline bool Widget::isMBtnLeftDown() const noexcept			{ return isActive() && context.mouseLDown; }
		inline bool Widget::wasPressed() const noexcept				{ return context.mouseLDownOld || (isHovered() && pressedOld); }

		inline const std::vector<sf::Event>& Widget::getEventsToPoll() const noexcept { return context.eventsToPoll; }
	}
}

#endif

