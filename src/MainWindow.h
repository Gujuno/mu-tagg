#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QVector>
#include <QString>
#include <QMap>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QMenuBar>
#include <QAction>
#include <QWidgetAction>
#include <QFileDialog>

struct AudioFileInfo {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    QString year;
    QString track;
    QString genre;
    QString comment;
    QString albumArtist;
    QString composer;
    QString disk;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSaveClicked();
    void onTableSelectionChanged();
    void onTagEditorFieldChanged();
    void onCommentEditFinished();
    void onTitleEditFinished();
    void onArtistEditFinished();
    void onAlbumEditFinished();
    void onYearEditFinished();
    void onTrackEditFinished();
    void onGenreEditFinished();
    void onAlbumArtistEditFinished();
    void onComposerEditFinished();
    void onDiskEditFinished();
    void onDelayedSaveConfig();
    void openFiles();
    void openDirectory();

private:
    void addFiles(const QStringList &filePaths);
    void loadTags(const QString &filePath, AudioFileInfo &info);
    void saveTags(const AudioFileInfo &info);
    void updateTagEditor(int row);
    void applyTagEditorToCurrentFile();
    void updateCoverArt(int row);
    void addOrRemoveCoverArt(bool add);
    void saveConfig();
    void loadConfig();
    void updateCoverArtSize();
    QStringList scanDirectoryRecursively(const QString &dirPath);
    void markUnsavedChanges();

    QTableWidget *table;
    QPushButton *saveButton;
    QVector<AudioFileInfo> audioFiles;
    bool updatingTable = false;

    // Tag editor widgets
    QWidget *tagEditorWidget;
    QLineEdit *titleEdit;
    QLineEdit *artistEdit;
    QLineEdit *albumEdit;
    QLineEdit *yearEdit;
    QLineEdit *trackEdit;
    QLineEdit *genreEdit;
    QTextEdit *commentEdit;
    QLineEdit *albumArtistEdit;
    QLineEdit *composerEdit;
    QLineEdit *diskEdit;
    bool updatingTagEditor = false;

    QLabel *coverArtLabel;
    QLabel *coverArtInfoLabel;
    QPixmap currentCoverPixmap;
    
    // Config save debouncing
    QTimer *configSaveTimer;
    
    // Unsaved changes tracking
    bool hasUnsavedChanges = false;
    
    // Menu bar
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
};
