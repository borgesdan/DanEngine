#ifndef DAN_VIEWPORT_WIDGET_HPP
#define DAN_VIEWPORT_WIDGET_HPP

#include <QWidget>

namespace Dan {

    class ViewportWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit ViewportWidget(QWidget* parent = nullptr);
        ~ViewportWidget() override = default;

        // Retorna o handle nativo (ex: HWND no Windows) para inicializar o bgfx
        void* GetNativeWindowHandle() const;

    protected:
        // Desativa a engine de pintura padrão do Qt para evitar conflitos (flickering)
        QPaintEngine* paintEngine() const override { return nullptr; }

        void paintEvent(QPaintEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
    };

} // namespace Dan

#endif