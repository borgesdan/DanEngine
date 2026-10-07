#include "ViewportWidget.hpp"
#include <QEvent>

namespace Dan {

    ViewportWidget::ViewportWidget(QWidget* parent) : QWidget(parent)
    {
        // Prepara o widget para receber renderização de uma API externa (bgfx)
        setAttribute(Qt::WA_PaintOnScreen, true);
        setAttribute(Qt::WA_NoSystemBackground, true);

        // Força o Qt a criar um handle nativo real para este widget
        setAttribute(Qt::WA_NativeWindow, true);
    }

    void* ViewportWidget::GetNativeWindowHandle() const
    {
        // winId() retorna o ponteiro nativo do sistema operacional que o bgfx exige
        return reinterpret_cast<void*>(winId());
    }

    void ViewportWidget::paintEvent(QPaintEvent* event)
    {
        // Deixamos vazio intencionalmente. A renderização será feita pelo bgfx na Engine.
    }

    void ViewportWidget::resizeEvent(QResizeEvent* event)
    {
        // Aqui você pode disparar um evento para a Engine atualizar a resolução do bgfx
    }

} // namespace Dan