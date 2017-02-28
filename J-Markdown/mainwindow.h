#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtWebKitWidgets/QWebView>
#include "CHtmlRender.h"
#include "CMarkdown.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(CHtmlRender render, QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void markdownHelp();
    void openRecentFile();
    void setDocumentModified();
    void markdownTran();
    void getStyle();

private:
    void createWorkPlace();
    void createActions();
    void createMenus();
    bool okToContinue();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);


    CMarkdown md;
    static QString head_beg, head_end, tail;
    QString style;
    QPlainTextEdit *in;
    QWebView *out;
    QLabel *statusLabel;
    QStringList recentFiles;
    QString curFile;
    bool isDocumentModified;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *separatorAction;
    QSplitter *mainSplitter;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *selectSubMenu;
    QMenu *toolsMenu;
    QMenu *optionsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *markdownHelpAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
};

#endif
