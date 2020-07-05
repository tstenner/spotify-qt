#pragma once

#include "icon.hpp"
#include "mainwindow.hpp"
#include "spotify/spotify.hpp"

#include <QDesktopServices>
#include <QMenu>

class SongMenu : public QMenu
{
public:
	SongMenu(const QString &trackId, const QString &artist, const QString &name,
		const QString &artistId, const QString &albumId, spt::Spotify *spotify, QWidget *parent = nullptr);

private:
	QVector<spt::Track> likedTracks;
	bool isLiked;
	spt::Spotify *spotify;
	const QString &trackId, &artistId, &albumId, &name, &artist;
	MainWindow *mainWindow;
	const spt::Playlist *currentPlaylist;
};