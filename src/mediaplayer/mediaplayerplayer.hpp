#pragma once

namespace mp { class MediaPlayerPlayer; }

#include "../spotify/spotify.hpp"
#include "../mainwindow.hpp"

#include <QDBusConnection>
#include <QDebug>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusAbstractAdaptor>
#include <QCoreApplication>
#include <QVariantMap>

namespace mp
{
	class MediaPlayerPlayer : public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

		Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
		Q_PROPERTY(double Rate READ playbackRate WRITE setPlaybackRate)
		Q_PROPERTY(QVariantMap Metadata READ metadata)
		Q_PROPERTY(double Volume READ getVolume WRITE setVolume)
		Q_PROPERTY(qint64 Position READ position)
		Q_PROPERTY(double MinimumRate READ playbackRate)
		Q_PROPERTY(double MaximumRate READ playbackRate)
		Q_PROPERTY(bool CanGoNext READ canControl)
		Q_PROPERTY(bool CanGoPrevious READ canControl)
		Q_PROPERTY(bool CanPlay READ canControl)
		Q_PROPERTY(bool CanPause READ canControl)
		Q_PROPERTY(bool CanSeek READ canControl)
		Q_PROPERTY(bool CanControl READ canControl)
		Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)

	public:
		explicit MediaPlayerPlayer(spt::Spotify *spotify, QObject *parent);

		bool canControl() const;
		QMap<QString, QVariant> metadata() const;

		double getVolume() const;
		void setVolume(double value) const;

		qint64 position() const;

		QString playbackStatus() const;

		double playbackRate() const;
		void setPlaybackRate(double value) const;

		bool shuffle() const;
		void setShuffle(bool value) const;

	signals:
		void Seeked(qint64 position);

	public slots:
		void Next() const;
		void Previous() const;
		void Pause() const;
		void PlayPause() const;
		void Stop() const;
		void Play() const;
		void Seek(qint64 offset) const;
		void SetPosition(QDBusObjectPath trackId, qint64 position) const;
		void OpenUri(QString uri) const;

	//private slots:
		void tick(qint64 newPos);
		void emitMetadataChange() const;
		void currentSourceChanged(const spt::Playback &playback) const;
		void stateUpdated() const;
		void totalTimeChanged(const spt::Playback &playback) const;
		void seekableChanged(bool seekable) const;
		void volumeChanged() const;

	private:
		QDBusConnection	dBus;
		spt::Spotify	*spotify;
	};
}