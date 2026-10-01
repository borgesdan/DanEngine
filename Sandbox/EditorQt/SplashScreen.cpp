#include "pch.hpp"
#include "SplashScreen.hpp"
#include "Version.hpp"

namespace Dan {
	SplashScreen::SplashScreen(const QPixmap& pixmap, Qt::WindowFlags f) : QSplashScreen(pixmap, f)
	{
		setWindowIcon(QIcon("icons:editor_icon.ico"));
		setWindowTitle("DANENGINE Sandbox");
		
		setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	}	

	void SplashScreen::SetVersion(const Version& v)
	{
		// O construtor .arg() é a forma moderna e idiomática do Qt6. 
		// É type-safe (à prova de falhas de tipo) e dinâmico.
		QString versionText = QString("Version %1.%2.%3 - Build %4")
			.arg(v.major)
			.arg(v.minor)
			.arg(v.patch)
			.arg(v.build);

		version = versionText;
		repaint();

		// qApp é a macro para a instância global do QApplication
		qApp->processEvents();
	}

	void SplashScreen::SetInfo(const char* text)
	{
		m_text = text;
		update();
		qApp->processEvents();
	}

	void SplashScreen::SetText(const char* szText)
	{
		SetInfo(szText);
	}

	void SplashScreen::drawContents(QPainter* painter)
	{
		QSplashScreen::drawContents(painter);
		painter->setPen(Qt::white);
		painter->drawText(rect().adjusted(30, 5, -5, -42), Qt::AlignLeft | Qt::AlignBottom, version);
		painter->drawText(rect().adjusted(30, 5, -5, -25), Qt::AlignLeft | Qt::AlignBottom, m_text);
	}
}
