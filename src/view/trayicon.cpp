#include "trayicon.hpp"
#include "mainwindow.hpp"

TrayIcon::TrayIcon(lib::spt::api &spotify, const lib::settings &settings,
	const lib::cache &cache, QWidget *parent)
	: QSystemTrayIcon(parent),
	spotify(spotify),
	settings(settings),
	cache(cache)
{
	callback = [this](const std::string &result)
	{
		message(QString::fromStdString(result));
	};

	contextMenu = new QMenu(parent);
	currentTrack = contextMenu->addAction("-");
	currentTrack->setEnabled(false);
	contextMenu->addSeparator();

	const auto previousIcon = Icon::get("media-skip-backward");
	previous = contextMenu->addAction(previousIcon, "Previous");
	QAction::connect(previous, &QAction::triggered,
		this, &TrayIcon::onPrevious);

	const auto playPauseIcon = Icon::get("media-playback-start");
	playPause = contextMenu->addAction(playPauseIcon, "Play");
	QAction::connect(playPause, &QAction::triggered,
		this, &TrayIcon::onPlayPause);

	next = contextMenu->addAction(Icon::get("media-skip-forward"), "Next");
	QAction::connect(next, &QAction::triggered,
		this, &TrayIcon::onNext);

	contextMenu->addSeparator();

#ifdef __APPLE__
	showApp = contextMenu->addAction(Icon::get("window"), QStringLiteral("Show"));
	QAction::connect(showApp, &QAction::triggered, this, &TrayIcon::onShowWindow);
#endif

	auto *quit = contextMenu->addAction(Icon::get("application-exit"), "Quit");
	QAction::connect(quit, &QAction::triggered, QCoreApplication::quit);

	setDefaultPixmap();
	setContextMenu(contextMenu);
	show();

#ifndef __APPLE__
	QSystemTrayIcon::connect(this, &QSystemTrayIcon::activated,
		this, &TrayIcon::onActivated);
#endif

	QMenu::connect(contextMenu, &QMenu::aboutToShow,
		this, &TrayIcon::onMenuAboutToShow);
}

TrayIcon::~TrayIcon()
{
	contextMenu->deleteLater();
}

void TrayIcon::message(const QString &message)
{
	if (message.isNull() || message.isEmpty())
	{
		return;
	}

	showMessage(APP_NAME, message);
}

void TrayIcon::message(const lib::spt::track &track, const QPixmap &pixmap)
{
	if (!track.is_valid())
	{
		return;
	}

	const auto title = QString::fromStdString(track.name);
	const auto artists = lib::spt::entity::combine_names(track.artists);
	const auto message = QString::fromStdString(artists);

	QIcon icon(pixmap);
	showMessage(title, message, icon, messageTrackTimeout);
}

auto TrayIcon::playback() -> lib::spt::playback
{
	auto *mainWindow = qobject_cast<MainWindow *>(this->parent());
	if (mainWindow == nullptr)
	{
		return {};
	}
	return mainWindow->currentPlayback();
}

void TrayIcon::setPixmap(const QPixmap &pixmap)
{
	setIcon(Image::mask(pixmap));
}

void TrayIcon::setDefaultPixmap()
{
	constexpr int iconSize = 64;

	setIcon(Icon::get(QString("logo:%1-symbolic-%2")
		.arg(APP_ICON)
		.arg(settings.general.tray_light_icon ? "light" : "dark"))
		.pixmap(iconSize, iconSize));
}

void TrayIcon::showWindow()
{
	auto *parentWidget = qobject_cast<QWidget *>(parent());
	if (parentWidget != nullptr)
	{
		parentWidget->setVisible(!parentWidget->isVisible());
	}
}

void TrayIcon::onPrevious(bool /*checked*/)
{
	spotify.previous(callback);
}

void TrayIcon::onPlayPause(bool /*checked*/)
{
	if (playback().is_playing)
	{
		spotify.pause(callback);
	}
	else
	{
		spotify.resume(callback);
	}
}

void TrayIcon::onNext(bool /*checked*/)
{
	spotify.next(callback);
}

void TrayIcon::onActivated(ActivationReason reason)
{
	if (reason == ActivationReason::Trigger)
	{
		showWindow();
	}
}

void TrayIcon::onShowWindow(bool /*checked*/)
{
	showWindow();
}

void TrayIcon::onMenuAboutToShow()
{
	auto current = playback();
	auto isPlaying = current.is_playing;

	currentTrack->setText(QString::fromStdString(current.item.title()));

	playPause->setIcon(Icon::get(isPlaying
		? "media-playback-pause"
		: "media-playback-start"));

	playPause->setText(isPlaying
		? "Pause"
		: "Play");

#ifdef __APPLE__
	if (showApp != nullptr)
	{
		auto *parentWidget = qobject_cast<QWidget *>(parent());
		if (parentWidget != nullptr)
		{
			showApp->setText(parentWidget->isVisible()
				? QStringLiteral("Hide")
				: QStringLiteral("Show"));
		}
	}
#endif
}
