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

		inline void Widget::updateInput()
		{
			// If the widget is not visible, do not process input
			if(!isVisible()) return;

			// Check hover - if true, set context.hovered to true
			hovered = isOverlapping(getMousePos(), 2.f);
			if(hovered) context.hovered = true;

			// Check if the widget is being pressed
			pressedLeft = isHovered() && context.mouseLDown;
			pressedRight = isHovered() && context.mouseRDown;

			// If the context is already busy with another widget, do not process input
			if(context.isBusy() && context.busyWith != this) return;

			auto processButton([this](bool mContextPressed, bool mPressed, ssvu::Delegate<void()>& onClick, ssvu::Delegate<void()>& onClickDown, ssvu::Delegate<void()>& onRelease)
			{
				if(mPressed)
				{
					if(isFocused() && (!onClick.isEmpty() || !onClickDown.isEmpty() || !onRelease.isEmpty()))
					{
						if(!context.isBusy())
						{
							onClick();
							context.busyWith = this;
						}
						else if(context.busyWith == this) onClickDown();
					}
				}
				else if(!mContextPressed && context.busyWith == this) onRelease();
			});

			processButton(context.mouseLDown, pressedLeft, onLeftClick, onLeftClickDown, onLeftRelease);
			processButton(context.mouseRDown, pressedRight, onRightClick, onRightClickDown, onRightRelease);
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

		inline const Vec2f& Widget::getMousePos() const noexcept						{ return context.mousePos; }
		inline const std::vector<sf::Event>& Widget::getEventsToPoll() const noexcept	{ return context.eventsToPoll; }
	}
}

#endif

