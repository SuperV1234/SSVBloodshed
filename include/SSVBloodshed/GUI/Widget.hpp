// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVOB_GUI_WIDGET
#define SSVOB_GUI_WIDGET

#include "SSVBloodshed/OBCommon.hpp"
#include "SSVBloodshed/GUI/At.hpp"
#include "SSVBloodshed/GUI/AABBShape.hpp"
#include "SSVBloodshed/GUI/Widget.hpp"

namespace ob
{
    namespace GUI
    {
        class Context;
        struct Style;

        class Widget : public AABBShape
        {
            friend Context;

        protected:
            Context& context;
            std::vector<Widget*> children;
            bool external{false}; // If true, this widget will not be taken into
                                  // account for scaling

            void render(const sf::Drawable& mDrawable);
            const std::vector<sf::Event>& getEventsToPoll() const noexcept;
            const Vec2f& getMousePos() const noexcept;
            bool isKeyPressed(ssvs::KKey mKey) const noexcept;
            const Style& getStyle() const noexcept;

        private:
            Widget* parent{nullptr};
            int depth{0};
            bool container{false}; // If true, children have a deeper depth
            sf::View view;
            Vec2f childBoundsMin, childBoundsMax, viewBoundsMin, viewBoundsMax;
            Vec2<bool> recalculated;

            // Settings
            bool hidden{
                false}; // Controlled by hide/show: if true, it makes the
                        // widget implicitly invisible and inactive
            bool excluded{
                false}; // Controlled by setExcludedRecursive: if true, it
                        // makes the widget implicitly invisible and
                        // inactive
            bool collapsed{
                false}; // Controlled by window collapsing: if true, it
                        // makes the widget implicitly invisible and
                        // inactive
            bool active{true}, visible{true};

            // Status
            bool focused{false}, hovered{false}, pressedLeft{false},
                pressedRight{false};

            // Positioning
            Widget* neighbor{nullptr};
            At from{At::Center}, to{At::Center};
            Vec2f offset;

            // Scaling
            Scaling scalingX{Scaling::Manual}, scalingY{Scaling::Manual};
            float padding{0.f}, scalePercent{100.f};

            inline virtual void update(FT) {}
            inline virtual void drawWidget() {}

            inline void drawHierarchy()
            {
                recurseChildrenBF([this](Widget& mW)
                    {
                        if(mW.isVisible())
                        {
                            mW.drawWidget();
                            mW.onPostDraw();
                            render(mW);
                        }
                    });
            }

            inline void setFocused(bool mValue) { focused = mValue; }

            template <typename TS, typename TG>
            inline void recalcSizeSmart(Scaling Widget::*mScaling, TS mSetter,
                TG mGetterMin, TG mGetterMax, float Vec2f::*mDimension,
                bool Vec2<bool>::*mDimensionBool)
            {
                if(recalculated.*mDimensionBool) return;
                recalculated.*mDimensionBool = true;

                if(neighbor != nullptr)
                {
                    neighbor->recalcSizeSmart(mScaling, mSetter, mGetterMin,
                        mGetterMax, mDimension, mDimensionBool);
                    setPosition(getVecPos(to, *neighbor) + offset +
                                (getPosition() - getVecPos(from, *this)));
                }

                if(this->*mScaling == Scaling::FitToChildren)
                {
                    childBoundsMin.*mDimension = childBoundsMax.*mDimension =
                                                     getPosition().*mDimension;

                    for(auto& w : children)
                    {
                        w->recalcSizeSmart(mScaling, mSetter, mGetterMin,
                            mGetterMax, mDimension, mDimensionBool);
                        if(!w->isVisible() || w->external) continue;

                        childBoundsMin.*mDimension = std::min(
                            childBoundsMin.*mDimension, (w->*mGetterMin)());
                        childBoundsMax.*mDimension = std::max(
                            childBoundsMax.*mDimension, (w->*mGetterMax)());
                    }

                    (this->*mSetter)(childBoundsMax.*mDimension -
                                     childBoundsMin.*mDimension +
                                     padding * 2.f);
                }
                else if(this->*mScaling == Scaling::FitToNeighbor)
                {
                    if(neighbor != nullptr)
                    {
                        neighbor->recalcSizeSmart(mScaling, mSetter, mGetterMin,
                            mGetterMax, mDimension, mDimensionBool);
                        (this->*mSetter)((((neighbor->*mGetterMax)() -
                                              (neighbor->*mGetterMin)()) *
                                             (scalePercent / 100.f)) -
                                         padding * 2.f);
                    }
                }
                else if(this->*mScaling == Scaling::FitToParent)
                {
                    if(parent != nullptr)
                    {
                        parent->recalcSizeSmart(mScaling, mSetter, mGetterMin,
                            mGetterMax, mDimension, mDimensionBool);
                        (this->*mSetter)((((parent->*mGetterMax)() -
                                              (parent->*mGetterMin)()) *
                                             (scalePercent / 100.f)) -
                                         padding * 2.f);
                    }
                }
            }

            inline void updateRecursive(FT mFT)
            {
                // Recalculate depth
                depth = parent == nullptr ? 0 : parent->depth +
                                                    ssvu::toInt(container);

                update(mFT);

                // Recalculate size and position
                recalcSizeSmart(&Widget::scalingX, &Widget::setWidth,
                    &Widget::getLeft, &Widget::getRight, &Vec2f::x,
                    &Vec2<bool>::x);
                recalcSizeSmart(&Widget::scalingY, &Widget::setHeight,
                    &Widget::getTop, &Widget::getBottom, &Vec2f::y,
                    &Vec2<bool>::y);

                for(auto& w : children) w->updateRecursive(mFT);

                recalculateView();
                onPostUpdate();

                updateInput();

                // not needed?
                // recurseChildrenBF<true, true>([this](Widget& mW){
                // mW.updateInput(); });
            }
            void recalculateView();
            void updateInput();

        public:
            using AABBShape::AABBShape;

            ssvu::Delegate<void()> onPostUpdate, onPostDraw;
            ssvu::Delegate<void()> onLeftClick, onLeftClickDown, onLeftRelease;
            ssvu::Delegate<void()> onRightClick, onRightClickDown,
                onRightRelease;

            inline Widget(Context& mContext) : context(mContext) {}
            inline Widget(Context& mContext, const Vec2f& mHalfSize)
                : AABBShape(ssvs::zeroVec2f, mHalfSize), context(mContext)
            {
            }
            inline Widget(Context& mContext, const Vec2f& mPosition,
                const Vec2f& mHalfSize)
                : AABBShape(mPosition, mHalfSize), context(mContext)
            {
            }
            inline virtual ~Widget() {}

            template <bool TIncludeCaller = true, typename T>
            inline void recurseChildren(const T& mFunc)
            {
                if(TIncludeCaller) mFunc(*this);
                for(const auto& w : children)
                    w->recurseChildren<true, T>(mFunc);
            }
            template <bool TIncludeCaller = true, bool TReverse = false,
                typename T>
            inline void recurseChildrenBF(const T& mFunc)
            {
                std::vector<Widget*> hierarchy;
                hierarchy.reserve(25);
                recurseChildren<TIncludeCaller>([&hierarchy](Widget& mW)
                    {
                        hierarchy.emplace_back(&mW);
                    });
                ssvu::sortStable(hierarchy,
                    [](const Widget* mA, const Widget* mB)
                    {
                        return (mA->depth < mB->depth) == !TReverse;
                    });
                for(const auto& w : hierarchy) mFunc(*w);
            }
            template <bool TIncludeCaller = true, typename T1, typename T2>
            inline void recurseChildrenIf(const T1& mPred, const T2& mFunc)
            {
                if(TIncludeCaller)
                {
                    if(!mPred(*this)) return;
                    mFunc(*this);
                }
                for(const auto& w : children)
                    w->recurseChildrenIf<true, T1, T2>(mPred, mFunc);
            }
            template <bool TIncludeCaller = true, typename T>
            inline void recurseParents(const T& mFunc)
            {
                if(TIncludeCaller) mFunc(*this);

                auto p(parent);
                while(p != nullptr)
                {
                    mFunc(*p);
                    p = p->parent;
                }
            }
            template <bool TIncludeCaller = true, typename T>
            inline bool SSVU_ATTRIBUTE(pure)
                isAnyChildRecursive(const T& mFunc) const
            {
                if(TIncludeCaller)
                    if(mFunc(*this)) return true;
                for(const auto& w : children)
                    if(w->isAnyChildRecursive<true, T>(mFunc)) return true;
                return false;
            }

            template <bool TIncludeCaller = true, typename T>
            inline bool SSVU_ATTRIBUTE(pure)
                isAnyParentRecursive(const T& mFunc) const
            {
                if(TIncludeCaller)
                    if(mFunc(*this)) return true;
                if(parent != nullptr)
                    if(parent->isAnyParentRecursive<true, T>(mFunc))
                        return true;
                return false;
            }

            inline void attach(At mFrom, Widget& mNeigh, At mTo,
                const Vec2f& mOffset = ssvs::zeroVec2f)
            {
                from = mFrom;
                neighbor = &mNeigh;
                to = mTo;
                offset = mOffset;
            }
            inline void show() { setHiddenRecursive(false); }
            inline void hide() { setHiddenRecursive(true); }

            // An hidden widget is both invisible and inactive (should be
            // controlled
            // by user)
            inline void setHidden(bool mValue) noexcept { hidden = mValue; }
            inline void setHiddenRecursive(bool mValue)
            {
                recurseChildren([mValue](Widget& mW)
                    {
                        mW.setHidden(mValue);
                    });
            }

            // An excluded widget is both invisible and inactive (should be used
            // to
            // completely disable a widget)
            inline void setExcluded(bool mValue) noexcept { excluded = mValue; }
            inline void setExcludedRecursive(bool mValue)
            {
                recurseChildren([mValue](Widget& mW)
                    {
                        mW.setExcluded(mValue);
                    });
            }
            inline void setExcludedSameDepth(bool mValue)
            {
                recurseChildrenIf(
                    [this](Widget& mW)
                    {
                        return mW.depth == depth;
                    },
                    [mValue](Widget& mW)
                    {
                        mW.setExcluded(mValue);
                    });
            }

            // A collapsed widget is both invisible and inactive (should be
            // controlled by collapsing windows)
            inline void setCollapsed(bool mValue) noexcept
            {
                collapsed = mValue;
            }
            inline void setCollapsedRecursive(bool mValue)
            {
                recurseChildren([mValue](Widget& mW)
                    {
                        mW.setCollapsed(mValue);
                    });
            }

            inline void setActiveRecursive(bool mValue)
            {
                active = mValue;
                for(auto& w : children) w->setActiveRecursive(mValue);
            }
            inline void setVisibleRecursive(bool mValue)
            {
                visible = mValue;
                for(auto& w : children) w->setVisibleRecursive(mValue);
            }
            inline void setContainer(bool mValue) { container = mValue; }
            inline void setParent(Widget& mWidget)
            {
                if(parent != nullptr) ssvu::eraseRemove(parent->children, this);

                parent = &mWidget;
                mWidget.children.emplace_back(this);

                setHiddenRecursive(mWidget.isHidden());
                setExcludedRecursive(mWidget.isExcluded());
                setCollapsedRecursive(mWidget.isCollapsed());
                setActiveRecursive(mWidget.isActive());
                setVisibleRecursive(mWidget.isVisible());
            }
            inline void setScalingX(Scaling mValue) noexcept
            {
                scalingX = mValue;
            }
            inline void setScalingY(Scaling mValue) noexcept
            {
                scalingY = mValue;
            }
            inline void setScaling(Scaling mValue) noexcept
            {
                scalingX = scalingY = mValue;
            }
            inline void setPadding(float mValue) noexcept { padding = mValue; }
            inline void setScalePercent(float mValue) noexcept
            {
                scalePercent = mValue;
            }
            inline void setExternal(bool mValue) noexcept { external = mValue; }

            inline bool isFocused() const noexcept { return focused; }
            inline bool isHovered() const noexcept
            {
                return isActive() && hovered;
            }
            inline bool isVisible() const noexcept
            {
                return visible && !isHidden() && !isExcluded() &&
                       !isCollapsed();
            }
            inline bool isActive() const noexcept
            {
                return active && !isHidden() && !isExcluded() && !isCollapsed();
            }
            inline bool isPressedAny() const noexcept
            {
                return pressedLeft || pressedRight;
            }
            inline bool isHidden() const noexcept { return hidden; }
            inline bool isExcluded() const noexcept { return excluded; }
            inline bool isCollapsed() const noexcept { return collapsed; }
            inline bool isContainer() const noexcept { return container; }

            inline float getPadding() const noexcept { return padding; }
            inline Scaling getScalingX() const noexcept { return scalingX; }
            inline Scaling getScalingY() const noexcept { return scalingY; }

            template <typename T, typename... TArgs>
            T& create(TArgs&&... mArgs);
            void destroyRecursive();
            void gainExclusiveFocus();
        };
    }
}

#endif
