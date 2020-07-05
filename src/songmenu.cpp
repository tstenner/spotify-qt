#include "songmenu.hpp"

SongMenu::SongMenu(const QString &trackId, const QString &artist, const QString &name,
		const QString &artistId, const QString &albumId, spt::Spotify *spotify, QWidget *parent)
		: trackId(trackId), spotify(spotify), artistId(artistId), albumId(albumId), name(name),
			artist(artist), QMenu(parent)
{
	mainWindow = dynamic_cast<MainWindow*>(parent);
	if (mainWindow == nullptr)
		return;
	auto track = trackId.startsWith("spotify:track")
		? QString(trackId).remove(0, QString("spotify:track:").length())
		: trackId;
	auto trackFeatures = addAction(Icon::get("view-statistics"), "Audio features");
	QAction::connect(trackFeatures, &QAction::triggered, [=](bool checked) {
		mainWindow->openAudioFeaturesWidget(trackId, artist, name);
	});
	auto lyrics = addAction(Icon::get("view-media-lyrics"), "Lyrics");
	QAction::connect(lyrics, &QAction::triggered, [=](bool checked) {
		mainWindow->openLyrics(artist, name);
	});
	auto share = addMenu(Icon::get("document-share"), "Share");
	auto shareSongLink = share->addAction("Copy song link");
	QAction::connect(shareSongLink, &QAction::triggered, [=](bool checked) {
		QApplication::clipboard()->setText(
			QString("https://open.spotify.com/track/%1").arg(track));
		mainWindow->setStatus("Link copied to clipboard");
	});
	auto shareSongOpen = share->addAction("Open in Spotify");
	QAction::connect(shareSongOpen, &QAction::triggered, [=](bool checked) {
		QDesktopServices::openUrl(QString("https://open.spotify.com/track/%1").arg(track));
	});
	addSeparator();
	// Add/remove liked
	likedTracks = mainWindow->loadTracksFromCache("liked");
	isLiked = false;
	for (auto i = 0; i < likedTracks.length(); i++)
		if (likedTracks.at(i).id == track)
		{
			isLiked = true;
			break;
		}
	auto toggleLiked = addAction(
		Icon::get(isLiked ? "starred-symbolic" : "non-starred-symbolic"),
		isLiked ? "Dislike" : "Like");
	QAction::connect(toggleLiked, &QAction::triggered, [this](bool checked) {
		auto status = isLiked
			? this->spotify->removeSavedTrack(this->trackId)
			: this->spotify->addSavedTrack(this->trackId);
		if (!status.isEmpty())
			mainWindow->setStatus(QString("Failed to %1: %2")
				.arg(isLiked ? "dislike" : "like")
				.arg(status), true);
	});
	// Add to queue
	auto addQueue = addAction(Icon::get("media-playlist-append"), "Add to queue");
	QAction::connect(addQueue, &QAction::triggered, [this](bool checked) {
		auto status = this->spotify->addToQueue(this->trackId.startsWith("spotify:track")
			? this->trackId
			: QString("spotify:track:%1").arg(this->trackId));
		if (!status.isEmpty())
			mainWindow->setStatus(status, true);
	});
	// Add to playlist
	addSeparator();
	auto addPlaylist = addMenu(Icon::get("list-add"), "Add to playlist");
	currentPlaylist = !mainWindow->hasPlaylistSelected()
		? nullptr
		: &mainWindow->sptPlaylists->at(mainWindow->playlists->currentRow());
	for (auto &playlist : *mainWindow->sptPlaylists)
	{
		// Create main action
		auto action = addPlaylist->addAction(playlist.name);
		action->setData(playlist.id);
	}
	QMenu::connect(addPlaylist, &QMenu::triggered, [this](QAction *action) {
		// Check if it's already in the playlist
		auto playlistId = action->data().toString();
		auto tracks = mainWindow->playlistTracks(playlistId);
		for (auto &track : tracks)
		{
			if (this->trackId.endsWith(track.id))
			{
				if (QMessageBox::information(this,
					"Duplicate",
					"Track is already in the playlist, do you want to add it anyway?",
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
					== QMessageBox::No)
					return;
				break;
			}
		}
		// Actually add
		auto plTrack = this->trackId.startsWith("spotify:track")
			? this->trackId
			: QString("spotify:track:%1").arg(this->trackId);
		auto result = this->spotify->addToPlaylist(playlistId, plTrack);
		if (!result.isEmpty())
			mainWindow->setStatus(QString("Failed to add track to playlist: %1").arg(result), true);
	});
	// Remove from playlist
	auto remPlaylist = addAction(Icon::get("list-remove"), "Remove from playlist");
	remPlaylist->setEnabled(mainWindow->playlists->currentRow() >= 0);
	QAction::connect(remPlaylist, &QAction::triggered, [this](bool checked) {
		// Remove from interface
		QTreeWidgetItem *item = nullptr;
		int i;
		for (i = 0; i < mainWindow->songs->topLevelItemCount(); i++)
		{
			item = mainWindow->songs->topLevelItem(i);
			if (item->data(0, MainWindow::RoleTrackId).toString() == this->trackId)
				break;
			// Failed to find
			item = nullptr;
		}
		if (item == nullptr)
		{
			mainWindow->setStatus("Failed to remove track, not found in playlist", true);
			return;
		}
		// Remove from Spotify
		auto status = this->spotify->removeFromPlaylist(currentPlaylist->id, this->trackId,
			item->data(0, MainWindow::RoleIndex).toInt());
		// Update status
		if (!status.isEmpty())
		{
			mainWindow->setStatus(QString("Failed to remove track from playlist: %1").arg(status), true);
			return;
		}
		// i doesn't necessarily match item index depending on sorting order
		mainWindow->songs->takeTopLevelItem(i);
		mainWindow->setStatus(QString(R"(Removed "%1 - %2" from "%3")")
			.arg(this->name).arg(this->artist).arg(currentPlaylist->name));
	});
	addSeparator();
	auto goArtist = addAction(Icon::get("view-media-artist"), "View artist");
	QAction::connect(goArtist, &QAction::triggered, [this](bool checked) {
		mainWindow->openArtist(this->artistId);
	});
	auto goAlbum = addAction(Icon::get("view-media-album-cover"), "Open album");
	QAction::connect(goAlbum, &QAction::triggered, [this](bool checked) {
		mainWindow->loadAlbum(this->albumId, false);
	});
}