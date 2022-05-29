#include "view/context/smallcontent.hpp"
#include "view/context/abstractcontent.hpp"

Context::SmallContent::SmallContent(lib::spt::api &spotify, spt::Current &current,
	const lib::cache &cache, QWidget *parent)
	: AbstractContent(spotify, current, cache, parent)
{
	auto *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignBottom);

	album = new AlbumCover(this);
	album->setFixedSize(albumSize, albumSize);

	layout->addWidget(album);
	nowPlaying = new Context::NowPlaying(this);
	layout->addWidget(nowPlaying);

	reset();

	// Show menu when clicking
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QLabel::connect(this, &QWidget::customContextMenuRequested,
		this, &Context::SmallContent::onSongMenu);

	// Context doesn't make sense to resize vertically
	setFixedHeight(layout->minimumSize().height());
}
