#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <QSplitter>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QPalette>
#include <QShortcut>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/flacfile.h>
#include <QFile>
#include <QBuffer>
#include <QImageReader>
#include <taglib/apefile.h>
#include <taglib/mp4file.h>
#include <taglib/id3v2frame.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacpicture.h>
#include <QDebug>
#include <taglib/xiphcomment.h>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("mu-tagg");
    resize(1000, 600);

    fileMenu = menuBar()->addMenu("File");
    editMenu = menuBar()->addMenu("Edit");
    viewMenu = menuBar()->addMenu("View");
    
    // Style the menu bar with a darker grey shade for better text contrast
    menuBar()->setStyleSheet("QMenuBar { background-color: #404040; color: white; border-bottom: 1px solid #303030; } QMenuBar::item:selected { background-color: #505050; }");
    
    // Add Open Files and Open Directory actions to File menu
    QAction *openFilesAction = new QAction("Open Files...", this);
    openFilesAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openFilesAction);
    connect(openFilesAction, &QAction::triggered, this, &MainWindow::openFiles);

    QAction *openDirAction = new QAction("Open Directory...", this);
    fileMenu->addAction(openDirAction);
    connect(openDirAction, &QAction::triggered, this, &MainWindow::openDirectory);

    // --- Tag editor panel ---
    tagEditorWidget = new QWidget(this);
    QVBoxLayout *tagLayout = new QVBoxLayout(tagEditorWidget);
    
    // Create input fields
    titleEdit = new QLineEdit(tagEditorWidget);
    artistEdit = new QLineEdit(tagEditorWidget);
    albumEdit = new QLineEdit(tagEditorWidget);
    
    // Year, Track, Genre on one line
    yearEdit = new QLineEdit(tagEditorWidget);
    yearEdit->setMaximumWidth(60);
    trackEdit = new QLineEdit(tagEditorWidget);
    trackEdit->setMaximumWidth(60);
    genreEdit = new QLineEdit(tagEditorWidget);
    
    commentEdit = new QTextEdit(tagEditorWidget);
    commentEdit->setFixedHeight(40);
    albumArtistEdit = new QLineEdit(tagEditorWidget);
    composerEdit = new QLineEdit(tagEditorWidget);
    diskEdit = new QLineEdit(tagEditorWidget);
    diskEdit->setObjectName("diskEdit");
    diskEdit->setMaximumWidth(60);
    
    // Add label-field pairs with labels above fields
    QLabel *titleLabel = new QLabel("Title", tagEditorWidget);
    tagLayout->addWidget(titleLabel);
    tagLayout->addWidget(titleEdit);
    
    QLabel *artistLabel = new QLabel("Artist", tagEditorWidget);
    tagLayout->addWidget(artistLabel);
    tagLayout->addWidget(artistEdit);
    
    QLabel *albumLabel = new QLabel("Album", tagEditorWidget);
    tagLayout->addWidget(albumLabel);
    tagLayout->addWidget(albumEdit);
    
    // Year, Track, Genre with individual labels
    QLabel *yearLabel = new QLabel("Year", tagEditorWidget);
    QLabel *trackLabel = new QLabel("Track", tagEditorWidget);
    QLabel *genreLabel = new QLabel("Genre", tagEditorWidget);
    
    // Create a single horizontal layout for labels and fields together
    QWidget *yearTrackGenreContainer = new QWidget(tagEditorWidget);
    QHBoxLayout *ytgContainerLayout = new QHBoxLayout(yearTrackGenreContainer);
    
    // Year section
    QVBoxLayout *yearLayout = new QVBoxLayout();
    yearLayout->addWidget(yearLabel);
    yearLayout->addWidget(yearEdit);
    yearLayout->setSpacing(2);
    yearLayout->setContentsMargins(0,0,0,0);
    
    // Track section
    QVBoxLayout *trackLayout = new QVBoxLayout();
    trackLayout->addWidget(trackLabel);
    trackLayout->addWidget(trackEdit);
    trackLayout->setSpacing(2);
    trackLayout->setContentsMargins(0,0,0,0);
    
    // Genre section
    QVBoxLayout *genreLayout = new QVBoxLayout();
    genreLayout->addWidget(genreLabel);
    genreLayout->addWidget(genreEdit);
    genreLayout->setSpacing(2);
    genreLayout->setContentsMargins(0,0,0,0);
    
    // Add all sections to the container
    ytgContainerLayout->addLayout(yearLayout, 1);
    ytgContainerLayout->addLayout(trackLayout, 1);
    ytgContainerLayout->addLayout(genreLayout, 2);
    ytgContainerLayout->setContentsMargins(0,0,0,0);
    
    yearTrackGenreContainer->setLayout(ytgContainerLayout);
    
    tagLayout->addWidget(yearTrackGenreContainer);
    
    QLabel *commentLabel = new QLabel("Comment", tagEditorWidget);
    tagLayout->addWidget(commentLabel);
    tagLayout->addWidget(commentEdit);
    
    QLabel *albumArtistLabel = new QLabel("Album Artist", tagEditorWidget);
    tagLayout->addWidget(albumArtistLabel);
    tagLayout->addWidget(albumArtistEdit);
    
    QLabel *composerLabel = new QLabel("Composer", tagEditorWidget);
    tagLayout->addWidget(composerLabel);
    tagLayout->addWidget(composerEdit);
    
    QLabel *diskLabel = new QLabel("Disk #", tagEditorWidget);
    tagLayout->addWidget(diskLabel);
    tagLayout->addWidget(diskEdit);
    
    tagLayout->addStretch(1);
    // --- Cover art box ---
    coverArtLabel = new QLabel(tagEditorWidget);
    coverArtLabel->setAlignment(Qt::AlignCenter);
    coverArtLabel->setStyleSheet("border: 1px solid #666; background: #444;");
    coverArtLabel->setText("No Cover");
    coverArtLabel->installEventFilter(this);
    // Info label for image dimensions and size
    coverArtInfoLabel = new QLabel(tagEditorWidget);
    coverArtInfoLabel->setAlignment(Qt::AlignCenter);
    coverArtInfoLabel->setStyleSheet("color: #666; font-size: 10pt;");
    coverArtInfoLabel->setText("");
    // Add vertical spacer, cover art, info label, and bottom stretch
    tagLayout->addSpacing(24);
    tagLayout->addWidget(coverArtLabel, 0, Qt::AlignHCenter);
    tagLayout->addWidget(coverArtInfoLabel, 0, Qt::AlignHCenter);
    tagLayout->addSpacing(16);
    tagLayout->addStretch(1);

    // --- Table and Save button ---
    QWidget *tablePanel = new QWidget(this);
    QVBoxLayout *tableLayout = new QVBoxLayout(tablePanel);
    table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({"Track", "Title", "Artist", "Album", "Year", "Genre"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionsMovable(true);
    table->horizontalHeader()->setSectionsClickable(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);

    tableLayout->addWidget(table);
    saveButton = new QPushButton("Save Changes", this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch(1);
    tableLayout->addLayout(buttonLayout);
    tablePanel->setLayout(tableLayout);

    // --- Splitter ---
    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(tagEditorWidget);
    splitter->addWidget(tablePanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({300, 700});

    setCentralWidget(splitter);
    setAcceptDrops(true);

    // Add Ctrl+S shortcut for saving
    QShortcut *saveShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSaveClicked);

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]{ onTableSelectionChanged(); });
    connect(titleEdit, &QLineEdit::editingFinished, this, &MainWindow::onTagEditorFieldChanged);
    connect(artistEdit, &QLineEdit::editingFinished, this, &MainWindow::onArtistEditFinished);
    connect(albumEdit, &QLineEdit::editingFinished, this, &MainWindow::onTagEditorFieldChanged);
    connect(yearEdit, &QLineEdit::editingFinished, this, &MainWindow::onTagEditorFieldChanged);
    connect(trackEdit, &QLineEdit::editingFinished, this, &MainWindow::onTagEditorFieldChanged);
    connect(genreEdit, &QLineEdit::editingFinished, this, &MainWindow::onGenreEditFinished);
    commentEdit->installEventFilter(this);
    connect(albumArtistEdit, &QLineEdit::editingFinished, this, &MainWindow::onAlbumArtistEditFinished);
    connect(composerEdit, &QLineEdit::editingFinished, this, &MainWindow::onComposerEditFinished);
    connect(diskEdit, &QLineEdit::editingFinished, this, &MainWindow::onDiskEditFinished);
    
    // Initialize config save timer for debouncing
    configSaveTimer = new QTimer(this);
    configSaveTimer->setSingleShot(true);
    configSaveTimer->setInterval(500); // 500ms delay
    connect(configSaveTimer, &QTimer::timeout, this, &MainWindow::onDelayedSaveConfig);
    
    // Auto-save config when splitter or table header changes (debounced for splitter)
    connect(splitter, &QSplitter::splitterMoved, this, [this]() {
        configSaveTimer->stop();
        configSaveTimer->start();
        updateCoverArtSize();
    });
    connect(table->horizontalHeader(), &QHeaderView::sectionMoved, this, &MainWindow::saveConfig);
    connect(table->horizontalHeader(), &QHeaderView::sectionResized, this, &MainWindow::saveConfig);
    
    loadConfig();
    
    // Initial cover art size update
    QTimer::singleShot(0, this, &MainWindow::updateCoverArtSize);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls()) {
        QFileInfo info(url.toLocalFile());
        if (info.isDir()) {
            QDir dir(info.filePath());
            QStringList filters = {"*.mp3", "*.flac"};
            QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
            for (const QFileInfo &fileInfo : fileList) {
                files << fileInfo.filePath();
            }
        } else if (info.isFile()) {
            files << info.filePath();
        }
    }
    addFiles(files);
}

void MainWindow::addFiles(const QStringList &filePaths) {
    for (const QString &filePath : filePaths) {
        AudioFileInfo info;
        info.filePath = filePath;
        loadTags(filePath, info);
        audioFiles.append(info);

        updatingTable = true;
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(info.track));
        table->setItem(row, 1, new QTableWidgetItem(info.title));
        table->setItem(row, 2, new QTableWidgetItem(info.artist));
        table->setItem(row, 3, new QTableWidgetItem(info.album));
        table->setItem(row, 4, new QTableWidgetItem(info.year));
        table->setItem(row, 5, new QTableWidgetItem(info.genre));
        updatingTable = false;
    }
    if (table->rowCount() > 0) {
        table->setCurrentCell(0, 0);
        updateTagEditor(0);
    }
}

void MainWindow::loadTags(const QString &filePath, AudioFileInfo &info) {
    if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
        TagLib::FLAC::File flacFile(filePath.toStdString().c_str());
        if (flacFile.isValid() && flacFile.xiphComment()) {
            TagLib::Ogg::XiphComment *comment = flacFile.xiphComment();
            info.title = QString::fromStdWString(comment->title().toWString());
            info.artist = QString::fromStdWString(comment->artist().toWString());
            info.album = QString::fromStdWString(comment->album().toWString());
            info.year = comment->fieldListMap().contains("DATE") ? QString::fromStdWString(comment->fieldListMap()["DATE"].toString().toWString()) : "";
            info.genre = QString::fromStdWString(comment->genre().toWString());
            info.comment = QString::fromStdWString(comment->comment().toWString());
            info.albumArtist = comment->fieldListMap().contains("ALBUMARTIST") ? QString::fromStdWString(comment->fieldListMap()["ALBUMARTIST"].toString().toWString()) : "";
            info.composer = comment->fieldListMap().contains("COMPOSER") ? QString::fromStdWString(comment->fieldListMap()["COMPOSER"].toString().toWString()) : "";
            info.disk = comment->fieldListMap().contains("DISCNUMBER") ? QString::fromStdWString(comment->fieldListMap()["DISCNUMBER"].toString().toWString()) : "";
            qDebug() << "[loadTags] FLAC disc number loaded:" << info.disk;
            info.track = QString::number(flacFile.tag() ? flacFile.tag()->track() : 0);
            qDebug() << "[loadTags]" << filePath << "title:" << info.title;
        }
    } else {
        TagLib::FileRef f(filePath.toStdString().c_str());
        if (!f.isNull() && f.tag()) {
            TagLib::Tag *tag = f.tag();
            info.title = QString::fromStdWString(tag->title().toWString());
            qDebug() << "[loadTags]" << filePath << "title:" << info.title;
            info.artist = QString::fromStdWString(tag->artist().toWString());
            info.album = QString::fromStdWString(tag->album().toWString());
            info.year = QString::number(tag->year());
            info.genre = QString::fromStdWString(tag->genre().toWString());
            info.comment = QString::fromStdWString(tag->comment().toWString());
            info.albumArtist = "";
            info.composer = "";
            info.disk = "";
            // Track number and disc number: use file type specific API
            if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
                TagLib::MPEG::File mpegFile(filePath.toStdString().c_str());
                if (mpegFile.isValid() && mpegFile.tag()) {
                    info.track = QString::number(mpegFile.tag()->track());
                    // Load disc number from ID3v2
                    if (TagLib::ID3v2::Tag *id3v2Tag = mpegFile.ID3v2Tag()) {
                        TagLib::ID3v2::FrameList discFrames = id3v2Tag->frameListMap()["TPOS"];
                        if (!discFrames.isEmpty()) {
                            info.disk = QString::fromStdString(discFrames.front()->toString().to8Bit());
                        }
                    }
                }
            } else {
                info.track = "";
                // Try to load disc number from other file types
                if (TagLib::ID3v2::Tag *id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
                    TagLib::ID3v2::FrameList discFrames = id3v2Tag->frameListMap()["TPOS"];
                    if (!discFrames.isEmpty()) {
                        info.disk = QString::fromStdString(discFrames.front()->toString().to8Bit());
                    }
                }
            }
        }
    }
}

void MainWindow::onSaveClicked() {
    qDebug() << "[onSaveClicked] Starting save process for" << audioFiles.size() << "files";
    
    // Update all fields if they have focus (to capture any pending changes)
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        // Capture field values before they might get cleared by focus change
        QString currentTitle = titleEdit->text();
        QString currentArtist = artistEdit->text();
        QString currentAlbum = albumEdit->text();
        QString currentYear = yearEdit->text();
        QString currentTrack = trackEdit->text();
        QString currentGenre = genreEdit->text();
        QString currentComment = commentEdit->toPlainText();
        QString currentAlbumArtist = albumArtistEdit->text();
        QString currentComposer = composerEdit->text();
        QString currentDisk = diskEdit->text();
        
        qDebug() << "[onSaveClicked] Captured field values - title:" << currentTitle << "disk:" << currentDisk;
        
        // Check if any field has focus and update accordingly
        if (commentEdit->hasFocus()) {
            qDebug() << "[onSaveClicked] Comment field has focus, updating comment";
            onCommentEditFinished();
        } else if (titleEdit->hasFocus() || artistEdit->hasFocus() || albumEdit->hasFocus() || 
                   yearEdit->hasFocus() || trackEdit->hasFocus() || genreEdit->hasFocus() ||
                   albumArtistEdit->hasFocus() || composerEdit->hasFocus() || diskEdit->hasFocus()) {
            qDebug() << "[onSaveClicked] Other field has focus, updating fields";
            onTagEditorFieldChanged();
        } else {
            // Check if any field values differ from the first selected file
            const AudioFileInfo &firstInfo = audioFiles[selected.first().row()];
            if (currentComment != firstInfo.comment) {
                qDebug() << "[onSaveClicked] Comment text differs from file, updating comment";
                onCommentEditFinished();
            }
            
            // Check other fields for differences using captured values
            if (currentTitle != firstInfo.title || currentArtist != firstInfo.artist ||
                currentAlbum != firstInfo.album || currentYear != firstInfo.year ||
                currentTrack != firstInfo.track || currentGenre != firstInfo.genre ||
                currentAlbumArtist != firstInfo.albumArtist || currentComposer != firstInfo.composer ||
                currentDisk != firstInfo.disk) {
                qDebug() << "[onSaveClicked] Other fields differ from file, updating fields";
                onTagEditorFieldChanged();
            }
        }
    }
    
    for (int i = 0; i < audioFiles.size(); ++i) {
        const AudioFileInfo &info = audioFiles[i];
        qDebug() << "[onSaveClicked] Saving file" << i << ":" << info.filePath;
        qDebug() << "[onSaveClicked] File" << i << "values - title:" << info.title << "comment:" << info.comment;
        saveTags(info);
    }
    
    // Refresh the table display to show the saved state
    qDebug() << "[onSaveClicked] Refreshing table display";
    updatingTable = true;
    for (int i = 0; i < audioFiles.size(); ++i) {
        const AudioFileInfo &info = audioFiles[i];
        table->setItem(i, 0, new QTableWidgetItem(info.track));
        table->setItem(i, 1, new QTableWidgetItem(info.title));
        table->setItem(i, 2, new QTableWidgetItem(info.artist));
        table->setItem(i, 3, new QTableWidgetItem(info.album));
        table->setItem(i, 4, new QTableWidgetItem(info.year));
        table->setItem(i, 5, new QTableWidgetItem(info.genre));
    }
    updatingTable = false;
    
    qDebug() << "[onSaveClicked] Save process completed";
    hasUnsavedChanges = false;
    qDebug() << "[onSaveClicked] Cleared unsaved changes flag";
    // No pop-up after saving
}

void MainWindow::saveTags(const AudioFileInfo &info) {
    qDebug() << "[saveTags] Starting to save tags for:" << info.filePath;
    auto safe = [](const QString &val) { return !val.isEmpty() && val != "[Mixed]"; };
    qDebug() << "[saveTags] About to save comment for:" << info.filePath << "comment:" << info.comment;
    qDebug() << "[saveTags] Disc number value:" << info.disk << "safe:" << safe(info.disk);
    if (info.filePath.endsWith(".flac", Qt::CaseInsensitive)) {
        qDebug() << "[saveTags] Processing FLAC file";
        TagLib::FLAC::File flacFile(info.filePath.toStdString().c_str());
        if (flacFile.isValid() && flacFile.xiphComment()) {
            qDebug() << "[saveTags] FLAC file is valid, saving tags";
            TagLib::Ogg::XiphComment *comment = flacFile.xiphComment();
            if (safe(info.title)) { qDebug() << "[saveTags]" << info.filePath << "title:" << info.title; comment->setTitle(info.title.toStdWString()); }
            if (safe(info.artist)) comment->setArtist(info.artist.toStdWString());
            if (safe(info.album)) comment->setAlbum(info.album.toStdWString());
            if (safe(info.year)) comment->addField("DATE", info.year.toStdWString(), true);
            if (safe(info.genre)) comment->setGenre(info.genre.toStdWString());
            if (safe(info.comment)) {
                comment->setComment(info.comment.toStdWString());
            } else if (info.comment.isEmpty()) {
                // Explicitly clear the comment field
                comment->removeFields("COMMENT");
                qDebug() << "[saveTags] Clearing comment field for:" << info.filePath;
            } else {
                qDebug() << "[saveTags] Comment not saved (mixed or other reason) for:" << info.filePath << "comment:" << info.comment;
            }
            if (safe(info.albumArtist)) comment->addField("ALBUMARTIST", info.albumArtist.toStdWString(), true);
            if (safe(info.composer)) comment->addField("COMPOSER", info.composer.toStdWString(), true);
            if (safe(info.disk)) {
                comment->addField("DISCNUMBER", info.disk.toStdWString(), true);
                qDebug() << "[saveTags] FLAC: Set disc number to:" << info.disk;
            } else {
                comment->removeFields("DISCNUMBER");
                qDebug() << "[saveTags] FLAC: Clearing disc number field for:" << info.filePath;
            }
            bool ok = false;
            uint trackNum = info.track.toUInt(&ok);
            if (ok && safe(info.track)) flacFile.tag()->setTrack(trackNum);
            bool saveResult = flacFile.save();
            qDebug() << "[saveTags] FLAC save result:" << saveResult;
        } else {
            qDebug() << "[saveTags] FLAC file is not valid or has no xiph comment";
        }
    } else {
        qDebug() << "[saveTags] Processing non-FLAC file";
        if (info.filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
            // Use MPEG-specific API for MP3 files to ensure all tags are saved together
            TagLib::MPEG::File mpegFile(info.filePath.toStdString().c_str());
            if (mpegFile.isValid() && mpegFile.tag()) {
                qDebug() << "[saveTags] MP3 file is valid, saving tags";
                TagLib::Tag *tag = mpegFile.tag();
                if (safe(info.title)) { qDebug() << "[saveTags]" << info.filePath << "title:" << info.title; tag->setTitle(info.title.toStdWString()); }
                if (safe(info.artist)) tag->setArtist(info.artist.toStdWString());
                if (safe(info.album)) tag->setAlbum(info.album.toStdWString());
                if (safe(info.year)) tag->setYear(info.year.toUInt());
                if (safe(info.genre)) tag->setGenre(info.genre.toStdWString());
                if (safe(info.comment)) {
                    tag->setComment(info.comment.toStdWString());
                } else if (info.comment.isEmpty()) {
                    // Explicitly clear the comment field
                    tag->setComment("");
                    qDebug() << "[saveTags] Clearing comment field for:" << info.filePath;
                } else {
                    qDebug() << "[saveTags] Comment not saved (mixed or other reason) for:" << info.filePath << "comment:" << info.comment;
                }
                
                // Track number
                bool ok = false;
                uint trackNum = info.track.toUInt(&ok);
                if (ok && safe(info.track)) {
                    tag->setTrack(trackNum);
                    qDebug() << "[saveTags] Set track number to:" << trackNum;
                }
                
                // Disc number - use ID3v2 frame for MP3
                if (safe(info.disk)) {
                    TagLib::ID3v2::Tag *id3v2Tag = mpegFile.ID3v2Tag();
                    if (id3v2Tag) {
                        TagLib::ID3v2::FrameList discFrames = id3v2Tag->frameListMap()["TPOS"];
                        if (!discFrames.isEmpty()) {
                            discFrames.front()->setText(info.disk.toStdString());
                        } else {
                            TagLib::ID3v2::TextIdentificationFrame *discFrame = new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::Latin1);
                            discFrame->setText(info.disk.toStdString());
                            id3v2Tag->addFrame(discFrame);
                        }
                        qDebug() << "[saveTags] Set disc number to:" << info.disk;
                    }
                } else if (info.disk.isEmpty()) {
                    // Explicitly clear the disc number field
                    TagLib::ID3v2::Tag *id3v2Tag = mpegFile.ID3v2Tag();
                    if (id3v2Tag) {
                        id3v2Tag->removeFrames("TPOS");
                        qDebug() << "[saveTags] Clearing disc number field for:" << info.filePath;
                    }
                }
                
                bool saveResult = mpegFile.save();
                qDebug() << "[saveTags] MP3 save result:" << saveResult;
            } else {
                qDebug() << "[saveTags] MP3 file is not valid or has no tag";
            }
        } else {
            // Use generic FileRef for other file types
            TagLib::FileRef f(info.filePath.toStdString().c_str());
            if (!f.isNull() && f.tag()) {
                qDebug() << "[saveTags] File is valid, saving tags";
                TagLib::Tag *tag = f.tag();
                if (safe(info.title)) { qDebug() << "[saveTags]" << info.filePath << "title:" << info.title; tag->setTitle(info.title.toStdWString()); }
                if (safe(info.artist)) tag->setArtist(info.artist.toStdWString());
                if (safe(info.album)) tag->setAlbum(info.album.toStdWString());
                if (safe(info.year)) tag->setYear(info.year.toUInt());
                if (safe(info.genre)) tag->setGenre(info.genre.toStdWString());
                if (safe(info.comment)) {
                    tag->setComment(info.comment.toStdWString());
                } else if (info.comment.isEmpty()) {
                    // Explicitly clear the comment field
                    tag->setComment("");
                    qDebug() << "[saveTags] Clearing comment field for:" << info.filePath;
                } else {
                    qDebug() << "[saveTags] Comment not saved (mixed or other reason) for:" << info.filePath << "comment:" << info.comment;
                }
                
                // Track number
                bool ok = false;
                uint trackNum = info.track.toUInt(&ok);
                if (ok && safe(info.track)) {
                    tag->setTrack(trackNum);
                    qDebug() << "[saveTags] Set track number to:" << trackNum;
                }
                
                // Disc number - try to save using available methods for other file types
                if (safe(info.disk)) {
                    // For other file types, we'll try to use ID3v2 if available
                    if (TagLib::ID3v2::Tag *id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
                        TagLib::ID3v2::FrameList discFrames = id3v2Tag->frameListMap()["TPOS"];
                        if (!discFrames.isEmpty()) {
                            discFrames.front()->setText(info.disk.toStdString());
                        } else {
                            TagLib::ID3v2::TextIdentificationFrame *discFrame = new TagLib::ID3v2::TextIdentificationFrame("TPOS", TagLib::String::Latin1);
                            discFrame->setText(info.disk.toStdString());
                            id3v2Tag->addFrame(discFrame);
                        }
                        qDebug() << "[saveTags] Set disc number to:" << info.disk;
                    } else {
                        qDebug() << "[saveTags] Could not save disc number - no ID3v2 tag available for file type";
                    }
                } else if (info.disk.isEmpty()) {
                    // Explicitly clear the disc number field
                    if (TagLib::ID3v2::Tag *id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
                        id3v2Tag->removeFrames("TPOS");
                        qDebug() << "[saveTags] Clearing disc number field for:" << info.filePath;
                    }
                }
                
                bool saveResult = f.file()->save();
                qDebug() << "[saveTags] Non-MP3 save result:" << saveResult;
            } else {
                qDebug() << "[saveTags] File is null or has no tag";
            }
        }
    }
    qDebug() << "[saveTags] Finished saving tags for:" << info.filePath;
}

void MainWindow::onTableSelectionChanged() {
    qDebug() << "[onTableSelectionChanged] Called";
    // Support multi-row selection
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        updateTagEditor(-1);
        return;
    }
    if (selected.size() == 1) {
        updateTagEditor(selected.first().row());
        return;
    }
    // Multi-row: show [Mixed] if values differ, empty if all empty
    updatingTagEditor = true;
    
    // Don't reset fields that are currently being edited
    bool titleHasFocus = titleEdit->hasFocus();
    bool artistHasFocus = artistEdit->hasFocus();
    bool albumHasFocus = albumEdit->hasFocus();
    bool yearHasFocus = yearEdit->hasFocus();
    bool trackHasFocus = trackEdit->hasFocus();
    bool genreHasFocus = genreEdit->hasFocus();
    bool commentHasFocus = commentEdit->hasFocus();
    bool albumArtistHasFocus = albumArtistEdit->hasFocus();
    bool composerHasFocus = composerEdit->hasFocus();
    bool diskHasFocus = diskEdit->hasFocus();
    
    qDebug() << "[onTableSelectionChanged] Field focus states - title:" << titleHasFocus << "disk:" << diskHasFocus;
    auto getField = [&](auto getter) {
        QString firstValue;
        bool firstSet = false;
        bool allEmpty = true;
        bool mixed = false;
        for (const QModelIndex &idx : selected) {
            QString val = getter(audioFiles[idx.row()]);
            if (!val.isEmpty()) {
                if (!firstSet) {
                    firstValue = val;
                    firstSet = true;
                } else if (val != firstValue) {
                    mixed = true;
                }
                allEmpty = false;
            }
        }
        if (allEmpty) return QString("");
        if (mixed) return QString("[Mixed]");
        return firstValue;
    };
    auto setEdit = [](QLineEdit *edit, const QString &val, bool hasFocus) {
        qDebug() << "[setEdit] Setting field to:" << val << "for field:" << edit->objectName() << "hasFocus:" << hasFocus;
        // Don't reset fields that are currently being edited
        if (hasFocus) {
            qDebug() << "[setEdit] Skipping field" << edit->objectName() << "because it has focus";
            return;
        }
        if (val == "[Mixed]") {
            edit->clear();
            edit->setPlaceholderText("[Mixed]");
            QPalette p = edit->palette();
            p.setColor(QPalette::PlaceholderText, QColor(180,180,180));
            edit->setPalette(p);
        } else if (val.isEmpty()) {
            edit->clear();
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        } else {
            edit->setText(val);
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        }
    };
    auto setEditText = [](QTextEdit *edit, const QString &val) {
        if (val == "[Mixed]") {
            edit->clear();
            edit->setPlaceholderText("[Mixed]");
            QPalette p = edit->palette();
            p.setColor(QPalette::PlaceholderText, QColor(180,180,180));
            edit->setPalette(p);
        } else if (val.isEmpty()) {
            edit->clear();
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        } else {
            edit->setPlainText(val);
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        }
    };
    QString titleVal = getField([](const AudioFileInfo &a){return a.title;});
    QString commentVal = getField([](const AudioFileInfo &a){return a.comment;});
    QString diskVal = getField([](const AudioFileInfo &a){return a.disk;});
    qDebug() << "[onTableSelectionChanged] Setting editor fields - title:" << titleVal << "comment:" << commentVal << "disk:" << diskVal;
    
    setEdit(titleEdit, titleVal, titleHasFocus);
    setEdit(artistEdit, getField([](const AudioFileInfo &a){return a.artist;}), artistHasFocus);
    setEdit(albumEdit, getField([](const AudioFileInfo &a){return a.album;}), albumHasFocus);
    setEdit(yearEdit, getField([](const AudioFileInfo &a){return a.year;}), yearHasFocus);
    setEdit(trackEdit, getField([](const AudioFileInfo &a){return a.track;}), trackHasFocus);
    setEdit(genreEdit, getField([](const AudioFileInfo &a){return a.genre;}), genreHasFocus);
    setEditText(commentEdit, commentVal);
    setEdit(albumArtistEdit, getField([](const AudioFileInfo &a){return a.albumArtist;}), albumArtistHasFocus);
    setEdit(composerEdit, getField([](const AudioFileInfo &a){return a.composer;}), composerHasFocus);
    setEdit(diskEdit, getField([](const AudioFileInfo &a){return a.disk;}), diskHasFocus);
    updatingTagEditor = false;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == coverArtLabel && event->type() == QEvent::ContextMenu) {
        QMenu menu;
        QAction *addAct = menu.addAction("Add Cover Art...");
        QAction *removeAct = menu.addAction("Remove Cover Art");
        QAction *chosen = menu.exec(static_cast<QContextMenuEvent*>(event)->globalPos());
        if (chosen == addAct) {
            addOrRemoveCoverArt(true);
        } else if (chosen == removeAct) {
            addOrRemoveCoverArt(false);
        }
        return true;
    }
    if (obj == commentEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Comment edit focus out detected";
        onCommentEditFinished();
    }
    if (obj == titleEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Title edit focus out detected";
        onTitleEditFinished();
    }
    if (obj == artistEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Artist edit focus out detected";
        onArtistEditFinished();
    }
    if (obj == albumEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Album edit focus out detected";
        onAlbumEditFinished();
    }
    if (obj == yearEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Year edit focus out detected";
        onYearEditFinished();
    }
    if (obj == trackEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Track edit focus out detected";
        onTrackEditFinished();
    }
    if (obj == genreEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Genre edit focus out detected";
        onGenreEditFinished();
    }
    if (obj == albumArtistEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Album artist edit focus out detected";
        onAlbumArtistEditFinished();
    }
    if (obj == composerEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Composer edit focus out detected";
        onComposerEditFinished();
    }
    if (obj == diskEdit && event->type() == QEvent::FocusOut) {
        qDebug() << "[eventFilter] Disk edit focus out detected";
        onDiskEditFinished();
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onCommentEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newComment = commentEdit->toPlainText();
    qDebug() << "[onCommentEditFinished] Updating comment for" << selected.size() << "selected files to:" << newComment;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.comment = newComment;
        qDebug() << "[onCommentEditFinished] Updated comment for file:" << info.filePath << "to:" << info.comment;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 5, new QTableWidgetItem(info.genre));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onTitleEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newTitle = titleEdit->text();
    qDebug() << "[onTitleEditFinished] Updating title for" << selected.size() << "selected files to:" << newTitle;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.title = newTitle;
        qDebug() << "[onTitleEditFinished] Updated title for file:" << info.filePath << "to:" << info.title;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 1, new QTableWidgetItem(info.title));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onArtistEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newArtist = artistEdit->text();
    qDebug() << "[onArtistEditFinished] Updating artist for" << selected.size() << "selected files to:" << newArtist;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.artist = newArtist;
        qDebug() << "[onArtistEditFinished] Updated artist for file:" << info.filePath << "to:" << info.artist;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 2, new QTableWidgetItem(info.artist));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onAlbumEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newAlbum = albumEdit->text();
    qDebug() << "[onAlbumEditFinished] Updating album for" << selected.size() << "selected files to:" << newAlbum;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.album = newAlbum;
        qDebug() << "[onAlbumEditFinished] Updated album for file:" << info.filePath << "to:" << info.album;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 3, new QTableWidgetItem(info.album));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onYearEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newYear = yearEdit->text();
    qDebug() << "[onYearEditFinished] Updating year for" << selected.size() << "selected files to:" << newYear;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.year = newYear;
        qDebug() << "[onYearEditFinished] Updated year for file:" << info.filePath << "to:" << info.year;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 4, new QTableWidgetItem(info.year));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onTrackEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newTrack = trackEdit->text();
    qDebug() << "[onTrackEditFinished] Updating track for" << selected.size() << "selected files to:" << newTrack;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.track = newTrack;
        qDebug() << "[onTrackEditFinished] Updated track for file:" << info.track;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 0, new QTableWidgetItem(info.track));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onGenreEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newGenre = genreEdit->text();
    qDebug() << "[onGenreEditFinished] Updating genre for" << selected.size() << "selected files to:" << newGenre;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.genre = newGenre;
        qDebug() << "[onGenreEditFinished] Updated genre for file:" << info.filePath << "to:" << info.genre;
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 5, new QTableWidgetItem(info.genre));
        updatingTable = false;
    }
    markUnsavedChanges();
}

void MainWindow::onAlbumArtistEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newAlbumArtist = albumArtistEdit->text();
    qDebug() << "[onAlbumArtistEditFinished] Updating album artist for" << selected.size() << "selected files to:" << newAlbumArtist;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.albumArtist = newAlbumArtist;
        qDebug() << "[onAlbumArtistEditFinished] Updated album artist for file:" << info.filePath << "to:" << info.albumArtist;
    }
    markUnsavedChanges();
}

void MainWindow::onComposerEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newComposer = composerEdit->text();
    qDebug() << "[onComposerEditFinished] Updating composer for" << selected.size() << "selected files to:" << newComposer;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.composer = newComposer;
        qDebug() << "[onComposerEditFinished] Updated composer for file:" << info.filePath << "to:" << info.composer;
    }
    markUnsavedChanges();
}

void MainWindow::onDiskEditFinished() {
    if (updatingTagEditor) return;
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    QString newDisk = diskEdit->text();
    qDebug() << "[onDiskEditFinished] Updating disk for" << selected.size() << "selected files to:" << newDisk;
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        info.disk = newDisk;
        qDebug() << "[onDiskEditFinished] Updated disk for file:" << info.filePath << "to:" << info.disk;
    }
    markUnsavedChanges();
}

void MainWindow::updateCoverArt(int row) {
    QPixmap blank(coverArtLabel->size());
    blank.fill(QColor(200, 200, 200));
    coverArtLabel->setPixmap(blank);
    coverArtLabel->setText("");
    coverArtInfoLabel->setText("");
    if (row < 0 || row >= audioFiles.size()) return;
    const QString &filePath = audioFiles[row].filePath;
    QPixmap pix;
    QByteArray imgData;
    if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        TagLib::MPEG::File mpegFile(filePath.toStdString().c_str());
        if (mpegFile.isValid()) {
            TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag();
            if (id3v2tag) {
                TagLib::ID3v2::FrameList frames = id3v2tag->frameListMap()["APIC"];
                if (!frames.isEmpty()) {
                    auto *picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
                    if (picFrame) {
                        imgData = QByteArray(reinterpret_cast<const char *>(picFrame->picture().data()), picFrame->picture().size());
                        pix.loadFromData(imgData);
                    }
                }
            }
        }
    } else if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
        TagLib::FLAC::File flacFile(filePath.toStdString().c_str());
        if (flacFile.isValid()) {
            auto pics = flacFile.pictureList();
            if (!pics.isEmpty()) {
                imgData = QByteArray(reinterpret_cast<const char *>(pics.front()->data().data()), pics.front()->data().size());
                pix.loadFromData(imgData);
            }
        }
    }
    if (!pix.isNull()) {
        coverArtLabel->setPixmap(pix.scaled(coverArtLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        coverArtLabel->setText("");
        // Show image info
        QString info = QString("%1 x %2 px, %3 KB")
            .arg(pix.width())
            .arg(pix.height())
            .arg(imgData.size() / 1024);
        coverArtInfoLabel->setText(info);
    } else {
        QPixmap blank(coverArtLabel->size());
        blank.fill(QColor(200, 200, 200));
        coverArtLabel->setPixmap(blank);
        coverArtLabel->setText("");
        coverArtInfoLabel->setText("");
    }
}

void MainWindow::addOrRemoveCoverArt(bool add) {
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    if (add) {
        // Open file dialog in the directory of the first selected song
        QString initialDir;
        if (!selected.isEmpty()) {
            QFileInfo fi(audioFiles[selected.first().row()].filePath);
            initialDir = fi.absolutePath();
        }
        QString imgPath = QFileDialog::getOpenFileName(this, "Select Cover Art", initialDir, "Images (*.png *.jpg *.jpeg *.bmp)");
        if (imgPath.isEmpty()) return;
        QFile imgFile(imgPath);
        if (!imgFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Failed to open image file.");
            return;
        }
        QByteArray imgData = imgFile.readAll();
        imgFile.close();
        for (const QModelIndex &idx : selected) {
            QString filePath = audioFiles[idx.row()].filePath;
            if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
                TagLib::MPEG::File mpegFile(filePath.toStdString().c_str());
                if (mpegFile.isValid()) {
                    TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag(true);
                    if (id3v2tag) {
                        // Remove old APIC frames
                        TagLib::ID3v2::FrameList frames = id3v2tag->frameListMap()["APIC"];
                        for (auto *frame : frames) id3v2tag->removeFrame(frame);
                        // Add new cover
                        auto *picFrame = new TagLib::ID3v2::AttachedPictureFrame();
                        picFrame->setMimeType("image/jpeg"); // assume jpeg, could check
                        picFrame->setPicture(TagLib::ByteVector(imgData.data(), imgData.size()));
                        id3v2tag->addFrame(picFrame);
                        mpegFile.save();
                    }
                }
            } else if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
                TagLib::FLAC::File flacFile(filePath.toStdString().c_str());
                if (flacFile.isValid()) {
                    // Remove old pictures
                    auto pics = flacFile.pictureList();
                    for (auto *pic : pics) flacFile.removePicture(pic);
                    // Add new picture
                    auto *pic = new TagLib::FLAC::Picture();
                    pic->setMimeType("image/jpeg");
                    pic->setType(TagLib::FLAC::Picture::FrontCover);
                    pic->setData(TagLib::ByteVector(imgData.data(), imgData.size()));
                    flacFile.addPicture(pic);
                    flacFile.save();
                }
            }
        }
    } else {
        // Remove cover art from all selected
        for (const QModelIndex &idx : selected) {
            QString filePath = audioFiles[idx.row()].filePath;
            if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
                TagLib::MPEG::File mpegFile(filePath.toStdString().c_str());
                if (mpegFile.isValid()) {
                    TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag(true);
                    if (id3v2tag) {
                        TagLib::ID3v2::FrameList frames = id3v2tag->frameListMap()["APIC"];
                        for (auto *frame : frames) id3v2tag->removeFrame(frame);
                        mpegFile.save();
                    }
                }
            } else if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
                TagLib::FLAC::File flacFile(filePath.toStdString().c_str());
                if (flacFile.isValid()) {
                    auto pics = flacFile.pictureList();
                    for (auto *pic : pics) flacFile.removePicture(pic);
                    flacFile.save();
                }
            }
        }
    }
    // Refresh cover art display
    if (!selected.isEmpty()) updateCoverArt(selected.first().row());
}

void MainWindow::updateTagEditor(int row) {
    qDebug() << "[updateTagEditor] ENTER - row:" << row << "updatingTagEditor:" << updatingTagEditor;
    updatingTagEditor = true;
    if (row < 0 || row >= audioFiles.size()) {
        qDebug() << "[updateTagEditor] Clearing all fields (invalid row)";
        titleEdit->clear(); artistEdit->clear(); albumEdit->clear();
        yearEdit->clear(); trackEdit->clear(); genreEdit->clear();
        commentEdit->clear(); albumArtistEdit->clear(); composerEdit->clear(); diskEdit->clear();
        updateCoverArt(-1);
        // Reset placeholder text and palette for all fields
        auto resetEdit = [](QLineEdit *edit) {
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        };
        auto resetEditText = [](QTextEdit *edit) {
            edit->setPlaceholderText("");
            edit->setPalette(QPalette());
        };
        resetEdit(titleEdit);
        resetEdit(artistEdit);
        resetEdit(albumEdit);
        resetEdit(yearEdit);
        resetEdit(trackEdit);
        resetEdit(genreEdit);
        resetEdit(albumArtistEdit);
        resetEdit(composerEdit);
        resetEdit(diskEdit);
        resetEditText(commentEdit);
        updatingTagEditor = false;
        qDebug() << "[updateTagEditor] EXIT - cleared fields";
        return;
    }
    const AudioFileInfo &info = audioFiles[row];
    qDebug() << "[updateTagEditor] Setting fields for row" << row << "- title:" << info.title << "comment:" << info.comment;
    titleEdit->setText(info.title);
    artistEdit->setText(info.artist);
    albumEdit->setText(info.album);
    yearEdit->setText(info.year);
    trackEdit->setText(info.track);
    genreEdit->setText(info.genre);
    commentEdit->setPlainText(info.comment);
    albumArtistEdit->setText(info.albumArtist);
    composerEdit->setText(info.composer);
    diskEdit->setText(info.disk);
    updateCoverArt(row);
    updatingTagEditor = false;
    qDebug() << "[updateTagEditor] EXIT - fields set";
}

void MainWindow::onTagEditorFieldChanged() {
    qDebug() << "[onTagEditorFieldChanged] ENTER - updatingTagEditor:" << updatingTagEditor;
    if (updatingTagEditor) {
        qDebug() << "[onTagEditorFieldChanged] EXIT - blocked by updatingTagEditor";
        return;
    }
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        qDebug() << "[onTagEditorFieldChanged] EXIT - no selection";
        return;
    }
    
    qDebug() << "[onTagEditorFieldChanged] Updating" << selected.size() << "selected files";
    qDebug() << "[onTagEditorFieldChanged] Current editor values - title:" << titleEdit->text() << "comment:" << commentEdit->toPlainText();
    
    // Get current editor values - handle [Mixed] placeholder text
    auto getFieldValue = [](QLineEdit *edit) {
        QString text = edit->text();
        QString placeholder = edit->placeholderText();
        qDebug() << "[getFieldValue] Field:" << edit->objectName() << "text:" << text << "placeholder:" << placeholder;
        
        // If the field is empty and has [Mixed] placeholder, return [Mixed]
        if (text.isEmpty() && placeholder == "[Mixed]") {
            return QString("[Mixed]");
        }
        // If the field has text, return the text (even if it's the same as the original value)
        if (!text.isEmpty()) {
            return text;
        }
        // If the field is empty and has no placeholder, return empty string
        return text;
    };
    
    QString newTitle = getFieldValue(titleEdit);
    QString newArtist = getFieldValue(artistEdit);
    QString newAlbum = getFieldValue(albumEdit);
    QString newYear = getFieldValue(yearEdit);
    QString newTrack = getFieldValue(trackEdit);
    QString newGenre = getFieldValue(genreEdit);
    QString newComment = commentEdit->toPlainText();
    QString newAlbumArtist = getFieldValue(albumArtistEdit);
    QString newComposer = getFieldValue(composerEdit);
    QString newDisk = getFieldValue(diskEdit);
    
    qDebug() << "[onTagEditorFieldChanged] Extracted values - title:" << newTitle << "comment:" << newComment << "disk:" << newDisk;
    
    // For multi-selection, only update fields that have been explicitly changed
    if (selected.size() > 1) {
        // Check if we have any non-[Mixed] values that are different from the first selected item
        const AudioFileInfo &firstInfo = audioFiles[selected.first().row()];
        bool hasExplicitChanges = false;
        
        // Only consider it a change if the field is not [Mixed] AND not empty (which indicates [Mixed] state)
        if (newTitle != "[Mixed]" && !newTitle.isEmpty() && newTitle != firstInfo.title) hasExplicitChanges = true;
        if (newArtist != "[Mixed]" && !newArtist.isEmpty() && newArtist != firstInfo.artist) hasExplicitChanges = true;
        if (newAlbum != "[Mixed]" && !newAlbum.isEmpty() && newAlbum != firstInfo.album) hasExplicitChanges = true;
        if (newYear != "[Mixed]" && !newYear.isEmpty() && newYear != firstInfo.year) hasExplicitChanges = true;
        if (newTrack != "[Mixed]" && !newTrack.isEmpty() && newTrack != firstInfo.track) hasExplicitChanges = true;
        if (newGenre != "[Mixed]" && !newGenre.isEmpty() && newGenre != firstInfo.genre) hasExplicitChanges = true;
        if (newComment != "[Mixed]" && !newComment.isEmpty() && newComment != firstInfo.comment) hasExplicitChanges = true;
        if (newAlbumArtist != "[Mixed]" && !newAlbumArtist.isEmpty() && newAlbumArtist != firstInfo.albumArtist) hasExplicitChanges = true;
        if (newComposer != "[Mixed]" && !newComposer.isEmpty() && newComposer != firstInfo.composer) hasExplicitChanges = true;
        if (newDisk != "[Mixed]" && !newDisk.isEmpty() && newDisk != firstInfo.disk) hasExplicitChanges = true;
        
        if (!hasExplicitChanges) {
            qDebug() << "[onTagEditorFieldChanged] EXIT - no explicit changes in multi-selection";
            return;
        }
    }
    
    for (const QModelIndex &idx : selected) {
        AudioFileInfo &info = audioFiles[idx.row()];
        qDebug() << "[onTagEditorFieldChanged] Before update - file:" << info.filePath << "title:" << info.title << "track:" << info.track << "comment:" << info.comment;
        
        // Update fields - handle both setting values and clearing fields
        // Only update if the field is not [Mixed] and not empty (which indicates [Mixed] state)
        if (newTitle != "[Mixed]" && !newTitle.isEmpty()) {
            info.title = newTitle;
            qDebug() << "[onTagEditorFieldChanged] Updated title to:" << newTitle;
        }
        if (newArtist != "[Mixed]" && !newArtist.isEmpty()) {
            info.artist = newArtist;
            qDebug() << "[onTagEditorFieldChanged] Updated artist to:" << newArtist;
        }
        if (newAlbum != "[Mixed]" && !newAlbum.isEmpty()) {
            info.album = newAlbum;
            qDebug() << "[onTagEditorFieldChanged] Updated album to:" << newAlbum;
        }
        if (newYear != "[Mixed]" && !newYear.isEmpty()) {
            info.year = newYear;
            qDebug() << "[onTagEditorFieldChanged] Updated year to:" << newYear;
        }
        if (newTrack != "[Mixed]" && !newTrack.isEmpty()) {
            info.track = newTrack;
            qDebug() << "[onTagEditorFieldChanged] Updated track to:" << newTrack;
        }
        if (newGenre != "[Mixed]" && !newGenre.isEmpty()) {
            info.genre = newGenre;
            qDebug() << "[onTagEditorFieldChanged] Updated genre to:" << newGenre;
        }
        if (newComment != "[Mixed]" && !newComment.isEmpty()) {
            info.comment = newComment;
            qDebug() << "[onTagEditorFieldChanged] Updated comment to:" << newComment;
        }
        if (newAlbumArtist != "[Mixed]" && !newAlbumArtist.isEmpty()) {
            info.albumArtist = newAlbumArtist;
            qDebug() << "[onTagEditorFieldChanged] Updated albumArtist to:" << newAlbumArtist;
        }
        if (newComposer != "[Mixed]" && !newComposer.isEmpty()) {
            info.composer = newComposer;
            qDebug() << "[onTagEditorFieldChanged] Updated composer to:" << newComposer;
        }
        if (newDisk != "[Mixed]" && !newDisk.isEmpty()) {
            info.disk = newDisk;
            qDebug() << "[onTagEditorFieldChanged] Updated disk to:" << newDisk;
        }
        
        qDebug() << "[onTagEditorFieldChanged] After update - file:" << info.filePath << "title:" << info.title << "track:" << info.track << "comment:" << info.comment;
        
        // Update table for each row
        updatingTable = true;
        table->setItem(idx.row(), 0, new QTableWidgetItem(info.track));
        table->setItem(idx.row(), 1, new QTableWidgetItem(info.title));
        table->setItem(idx.row(), 2, new QTableWidgetItem(info.artist));
        table->setItem(idx.row(), 3, new QTableWidgetItem(info.album));
        table->setItem(idx.row(), 4, new QTableWidgetItem(info.year));
        table->setItem(idx.row(), 5, new QTableWidgetItem(info.genre));
        updatingTable = false;
    }
    markUnsavedChanges();
    qDebug() << "[onTagEditorFieldChanged] EXIT";
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete && table->hasFocus()) {
        QList<int> rows;
        for (const QModelIndex &idx : table->selectionModel()->selectedRows()) {
            rows.append(idx.row());
        }
        std::sort(rows.begin(), rows.end(), std::greater<int>()); // Remove from bottom up
        for (int row : rows) {
            table->removeRow(row);
            audioFiles.remove(row);
        }
        // Update tag editor to reflect new selection
        if (table->rowCount() > 0)
            updateTagEditor(table->currentRow());
        else
            updateTagEditor(-1);
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::saveConfig() {
    QSettings settings("mu-tagg", "mu-tagg");
    settings.setValue("geometry", saveGeometry());
    if (auto splitter = qobject_cast<QSplitter*>(centralWidget())) {
        QList<int> sizes = splitter->sizes();
        QStringList sizeStrings;
        for (int size : sizes) {
            sizeStrings << QString::number(size);
        }
        settings.setValue("splitterSizes", sizeStrings.join(","));
    }
    settings.setValue("tableHeaderState", table->horizontalHeader()->saveState());
}

void MainWindow::onDelayedSaveConfig() {
    saveConfig();
}

void MainWindow::loadConfig() {
    QSettings settings("mu-tagg", "mu-tagg");
    if (settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
    if (auto splitter = qobject_cast<QSplitter*>(centralWidget())) {
        QString sizeString = settings.value("splitterSizes").toString();
        if (!sizeString.isEmpty()) {
            QStringList sizeStrings = sizeString.split(",");
            QList<int> sizes;
            for (const QString &sizeStr : sizeStrings) {
                bool ok;
                int size = sizeStr.toInt(&ok);
                if (ok) sizes << size;
            }
            if (!sizes.isEmpty()) splitter->setSizes(sizes);
        }
    }
    if (settings.contains("tableHeaderState")) {
        table->horizontalHeader()->restoreState(settings.value("tableHeaderState").toByteArray());
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "Unsaved Changes", 
            "You have unsaved changes, do you wish to save?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            onSaveClicked();
            hasUnsavedChanges = false;
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainWindow::markUnsavedChanges() {
    hasUnsavedChanges = true;
    qDebug() << "[markUnsavedChanges] Marked unsaved changes";
}

void MainWindow::updateCoverArtSize() {
    if (!tagEditorWidget) return;
    
    // Calculate size based on panel width (50% bigger than before)
    int panelWidth = tagEditorWidget->width();
    int coverSize = qMin(panelWidth - 40, 240); // 50% bigger than 160, with some margin
    
    // Ensure minimum size
    coverSize = qMax(coverSize, 120);
    
    coverArtLabel->setFixedSize(coverSize, coverSize);
    
    // Update the current cover art if there is one
    if (!currentCoverPixmap.isNull()) {
        updateCoverArt(table->currentRow());
    }
}

void MainWindow::openFiles() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Audio Files (*.mp3 *.flac)");
    dialog.setWindowTitle("Open Files");
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            qDebug() << "[openFiles] Adding" << files.size() << "files to the application";
            addFiles(files);
        } else {
            qDebug() << "[openFiles] No audio files found";
        }
    }
}

void MainWindow::openDirectory() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Directory", QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        qDebug() << "[openDirectory] Scanning directory:" << dirPath;
        QStringList files = scanDirectoryRecursively(dirPath);
        if (!files.isEmpty()) {
            qDebug() << "[openDirectory] Adding" << files.size() << "files to the application";
            addFiles(files);
        } else {
            qDebug() << "[openDirectory] No audio files found in directory";
        }
    }
}

QStringList MainWindow::scanDirectoryRecursively(const QString &dirPath) {
    QStringList audioFiles;
    QDir dir(dirPath);
    
    qDebug() << "[scanDirectoryRecursively] Scanning directory:" << dirPath;
    
    // Get all files in current directory and check if they're audio files
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Name);
    for (const QFileInfo &fileInfo : fileList) {
        QString fileName = fileInfo.fileName().toLower();
        if (fileName.endsWith(".mp3") || fileName.endsWith(".flac")) {
            audioFiles << fileInfo.filePath();
            qDebug() << "[scanDirectoryRecursively] Found audio file:" << fileInfo.filePath();
        }
    }
    
    // Recursively scan subdirectories
    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    for (const QFileInfo &dirInfo : dirList) {
        qDebug() << "[scanDirectoryRecursively] Scanning subdirectory:" << dirInfo.filePath();
        QStringList subDirFiles = scanDirectoryRecursively(dirInfo.filePath());
        audioFiles.append(subDirFiles);
    }
    
    qDebug() << "[scanDirectoryRecursively] Found" << audioFiles.size() << "audio files in" << dirPath;
    return audioFiles;
}
