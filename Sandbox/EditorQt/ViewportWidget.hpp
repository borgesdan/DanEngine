#pragma once

#include <QWidget>
#include <QPoint>
#include <Dan/Common/Math/MathTypes.hpp>

namespace Dan {

    struct InputState;

    class ViewportWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit ViewportWidget(QWidget* parent = nullptr);
        ~ViewportWidget() override = default;

        void* GetNativeWindowHandle() const;

        // Injeta a referência para o estado de entrada do EngineContext
        void SetInputState(InputState* inputState);

    signals:
        // Sinal emitido ao clicar na Viewport para picking 3D por Raycasting
        void ViewportClicked(const Math::Vec2& mousePos, const Math::Vec2& viewportSize);
        void ViewportMouseReleased();

    protected:
        QPaintEngine* paintEngine() const override { return nullptr; }

        void paintEvent(QPaintEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

        // Sobrescrevendo manipuladores de eventos de entrada do Qt
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void focusOutEvent(QFocusEvent* event) override;

    private:
        static int QtKeyToSDLScancode(int qtKey);
        static int QtButtonToSDLButton(Qt::MouseButton button);

    private:
        InputState* m_inputState = nullptr;
        QPoint m_lastMousePos;
        bool m_isMouseTrackingActive = false;
    };

} // namespace Dan