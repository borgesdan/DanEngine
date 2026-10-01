#ifndef DAN_EDITORQT_SPLASHSCREEN_HPP
#define DAN_EDITORQT_SPLASHSCREEN_HPP

#include <QSplashScreen>
#include <QPixmap>
#include <memory>
#include <QPainter>
#include "Version.hpp"

namespace Dan {
	//Responsável por gerenciar o SplashScreen do Editor
	class SplashScreen final : public QSplashScreen
	{
	public:
		SplashScreen(const QPixmap& pixmap = QPixmap(), Qt::WindowFlags f = Qt::WindowType::Widget);

		void SetVersion(const Version& v);
		void SetText(const char* szText);

	private:
		void drawContents(QPainter* painter) override;
		void SetInfo(const char* text);

		QString version;
		QString m_text;
	};
}

#endif