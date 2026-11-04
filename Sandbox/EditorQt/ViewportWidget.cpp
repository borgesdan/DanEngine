#include "ViewportWidget.hpp"
#include <Dan/Common/Input/InputState.hpp>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include <Dan/Common/Math/MathTypes.hpp>

namespace Dan {

    ViewportWidget::ViewportWidget(QWidget* parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_PaintOnScreen, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_NativeWindow, true);

        // Garante que o Widget aceite foco do teclado e rastreie o mouse ao ser clicado
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
    }

    void ViewportWidget::SetInputState(InputState* inputState)
    {
        m_inputState = inputState;
        if (m_inputState) {
            m_inputState->InitForEditor();
        }
    }

    void* ViewportWidget::GetNativeWindowHandle() const
    {
        return reinterpret_cast<void*>(winId());
    }

    void ViewportWidget::paintEvent(QPaintEvent* event) {}

    void ViewportWidget::resizeEvent(QResizeEvent* event)
    {
        QWidget::resizeEvent(event);
    }

    void ViewportWidget::keyPressEvent(QKeyEvent* event)
    {
        if (!m_inputState || event->isAutoRepeat()) return;

        int scancode = QtKeyToSDLScancode(event->key());
        if (scancode != SDL_SCANCODE_UNKNOWN) {
            m_inputState->SetKeyDown(scancode, true);
        }
    }

    void ViewportWidget::keyReleaseEvent(QKeyEvent* event)
    {
        if (!m_inputState || event->isAutoRepeat()) return;

        int scancode = QtKeyToSDLScancode(event->key());
        if (scancode != SDL_SCANCODE_UNKNOWN) {
            m_inputState->SetKeyDown(scancode, false);
        }
    }

    void ViewportWidget::mousePressEvent(QMouseEvent* event)
    {
        setFocus();

        if (m_inputState) {
            int sdlButton = QtButtonToSDLButton(event->button());
            if (sdlButton != 0) {
                m_inputState->SetMouseButtonDown(sdlButton, true);
            }
        }

        // Clique com o Botão Esquerdo realiza a Seleção 3D por Raycasting[cite: 5, 10]
        if (event->button() == Qt::LeftButton)
        {
            QPoint localPos = event->pos();
            Math::Vec2 mousePos(static_cast<float>(localPos.x()), static_cast<float>(localPos.y()));
            Math::Vec2 viewportSize(static_cast<float>(width()), static_cast<float>(height()));

            emit ViewportClicked(mousePos, viewportSize);
        }

        m_lastMousePos = event->globalPosition().toPoint();
        m_isMouseTrackingActive = true;
    }

    void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        if (m_inputState) {
            int sdlButton = QtButtonToSDLButton(event->button());
            if (sdlButton != 0) {
                m_inputState->SetMouseButtonDown(sdlButton, false);
            }
        }

        if (event->button() == Qt::LeftButton) {
            emit ViewportMouseReleased();
        }

        m_isMouseTrackingActive = false;
    }

    void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if (!m_inputState) return;

        QPoint currentPos = event->globalPosition().toPoint();

        if (m_isMouseTrackingActive || m_inputState->mouseButtonState != 0) {
            QPoint delta = currentPos - m_lastMousePos;
            m_inputState->mouseDeltaX += static_cast<float>(delta.x());
            m_inputState->mouseDeltaY += static_cast<float>(delta.y());
        }

        m_lastMousePos = currentPos;
    }

    void ViewportWidget::focusOutEvent(QFocusEvent* event)
    {
        // Limpa o estado quando o editor perde o foco (evita teclas presas ao usar Alt+Tab)
        if (m_inputState) {
            m_inputState->localKeyBuffer.fill(false);
            m_inputState->mouseButtonState = 0;
            m_inputState->ClearFrameDeltas();
        }
        m_isMouseTrackingActive = false;
        QWidget::focusOutEvent(event);
    }

    int ViewportWidget::QtKeyToSDLScancode(int qtKey)
    {
        switch (qtKey) {
        case Qt::Key_W: return SDL_SCANCODE_W;
        case Qt::Key_A: return SDL_SCANCODE_A;
        case Qt::Key_S: return SDL_SCANCODE_S;
        case Qt::Key_D: return SDL_SCANCODE_D;
        case Qt::Key_Q: return SDL_SCANCODE_Q;
        case Qt::Key_E: return SDL_SCANCODE_E;
        case Qt::Key_R: return SDL_SCANCODE_R;
        case Qt::Key_F: return SDL_SCANCODE_F;
        case Qt::Key_Space: return SDL_SCANCODE_SPACE;
        case Qt::Key_Shift: return SDL_SCANCODE_LSHIFT;
        case Qt::Key_Control: return SDL_SCANCODE_LCTRL;
        case Qt::Key_Alt: return SDL_SCANCODE_LALT;
        case Qt::Key_Escape: return SDL_SCANCODE_ESCAPE;
        case Qt::Key_F5: return SDL_SCANCODE_F5;
        case Qt::Key_Up: return SDL_SCANCODE_UP;
        case Qt::Key_Down: return SDL_SCANCODE_DOWN;
        case Qt::Key_Left: return SDL_SCANCODE_LEFT;
        case Qt::Key_Right: return SDL_SCANCODE_RIGHT;
        default: break;
        }

        if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
            return SDL_SCANCODE_A + (qtKey - Qt::Key_A);
        }

        return SDL_SCANCODE_UNKNOWN;
    }

    int ViewportWidget::QtButtonToSDLButton(Qt::MouseButton button)
    {
        switch (button) {
        case Qt::LeftButton:   return SDL_BUTTON_LEFT;
        case Qt::RightButton:  return SDL_BUTTON_RIGHT;
        case Qt::MiddleButton: return SDL_BUTTON_MIDDLE;
        default:               return 0;
        }
    }

} // namespace Dan