#pragma once

#include <QWidget>
#include <QPoint>

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