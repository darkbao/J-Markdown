#include <QtGui>
#include "mainwindow.h"

QString MainWindow::head_beg = "<html><head><style type=\"text/css\">",
        MainWindow::head_end = "</style></head><body>",
        MainWindow::tail = "</body></html>";


MainWindow::MainWindow(CHtmlRender render, QWidget *parent)
    :QMainWindow(parent),md(render),style()
{
    createWorkPlace();
    createActions();
    createMenus();
    setCentralWidget(mainSplitter);
    setWindowIcon(QIcon(":/images/icon.png"));
    setCurrentFile("");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (okToContinue()) {
        in->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open Markdown"), ".",
                                   tr("Markdown files (*.md)"));
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty())
    {
        return saveAs();
    }
    else
    {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                               tr("Save Markdown"), ".",
                               tr("Markdown files (*.md)"));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::markdownHelp()
{
    QMessageBox help(QMessageBox::NoIcon, tr("About Markdown"),
                       tr("<h4>This website will show you the basic syntax of markdown:</h4>"
                          "CHS: <a href=\"https://github.com/othree/markdown-syntax-zhtw\">https://github.com/othree/markdown-syntax-zhtw</a><br>"
                          "En: <a href=\"http://daringfireball.net/projects/markdown/syntax#list\">http://daringfireball.net/projects/markdown/syntax#list</a>"));
    help.setIconPixmap(QPixmap(":/images/link.png"));
    help.setWindowIcon(QIcon(":/images/icon.png"));
    help.exec();
}

void MainWindow::about()
{
    QMessageBox message(QMessageBox::NoIcon, tr("About Markdown"),
                        tr("<h2>J-Markdown 1.0</h2>"
                           "<p>J-Markdown is a Markdown parser and also a small application that "
                           "satisfys all Syntax presented in: <a href=\"https://github.com/othree/markdown-syntax-zhtw\">https://github.com/othree/markdown-syntax-zhtw</a>"
                           ". It's free to use, copy, modify, and distribute this software for any "
                           "purpose with or without fee."
                           "<hr>"
                           "<h3>Project address</h3>"
                           "<p><a href=\"https://github.com/darkbao/Markdown\">https://github.com/darkbao/Markdown</a></p>"
                           "<hr>"
                           "<h3>Author</h3>"
                           "<p>Homepage: <a href=\"https://github.com/darkbao\">https://github.com/darkbao</a><p>"
                           "<p>E-mail: <a href=\"freecare_j@163.com\">freecare_j@163.com</a></p>"));
    message.setIconPixmap(QPixmap(":/images/about.png"));
    message.setWindowIcon(QIcon(":/images/icon.png"));
    message.exec();
}

void MainWindow::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadFile(action->data().toString());
    }
}


void MainWindow::createWorkPlace()
{
    mainSplitter = new QSplitter(Qt::Horizontal,this);
    in = new QPlainTextEdit(mainSplitter);
    auto font = QFont("Consolas",12);
    in->setFont(font);
    in->setTabStopWidth(40);
    out = new QWebView(mainSplitter);
    QList<int> tmp;
    tmp << 200 << 200;
    mainSplitter->setSizes(tmp);
    isDocumentModified = false;
    getStyle();
    connect(in,&QPlainTextEdit::modificationChanged,this,&MainWindow::setDocumentModified);
    connect(in,&QPlainTextEdit::textChanged,this,&MainWindow::markdownTran);
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setIcon(QIcon(":/images/new.png"));
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new Markdown file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(tr("&Open..."), this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing Markdown file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setIcon(QIcon(":/images/save.png"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the Markdown to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setStatusTip(tr("Save the Markdown under a new "
                                  "name"));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    markdownHelpAction = new QAction(tr("Markdown Syntax"), this);
    markdownHelpAction->setStatusTip(tr("Help you use markdown"));
    connect(markdownHelpAction, SIGNAL(triggered()), this, SLOT(markdownHelp()));


    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));


    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    separatorAction = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(markdownHelpAction);
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

bool MainWindow::okToContinue()
{
    if (isDocumentModified) {
        int r = QMessageBox::warning(this, tr("Markdown"),
                        tr("The document has been modified.\n"
                           "Do you want to save your changes?"),
                        QMessageBox::Yes | QMessageBox::No
                        | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return save();
        }
        else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

bool MainWindow::loadFile(const QString &fileName)
{
    QTextCodec *codec =QTextCodec::codecForName("UTF8");
    QTextCodec::setCodecForLocale(codec);
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,tr("Warnning"),tr("can't open file."),QMessageBox::Yes);
        return false;
    }
    auto byte = file.readAll();
    in->setPlainText(codec->toUnicode(byte));
    setCurrentFile(fileName);
    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Markdown"),
                             tr("Cannot write file."),QMessageBox::Yes);
        return false;
    }
    file.write(in->toPlainText().toStdString().c_str());
    isDocumentModified = false;
    setCurrentFile(fileName);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    isDocumentModified = false;

    QString shownName = tr("Untitled");
    if (!curFile.isEmpty()) {
        shownName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }

    setWindowTitle(tr("%1[*] - %2").arg(shownName)
                                   .arg(tr("Markdown")));
}

void MainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        }
        else {
            recentFileActions[j]->setVisible(false);
        }
    }
    separatorAction->setVisible(!recentFiles.isEmpty());
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::setDocumentModified()
{
    isDocumentModified=true;
}

void  MainWindow::markdownTran()
{
    QString head = head_beg + style + head_end;
    out->setHtml(head + QString::fromStdWString(md.tranform(in->toPlainText().toStdWString())) + tail);
}

void MainWindow::getStyle()
{
    QFile css_file(":/user.css");
    if(!css_file.open(QFile::ReadOnly)){
        QMessageBox::warning(this,tr("Warnning"),tr("can't open this CSS file."),QMessageBox::Yes);
    }
    QTextStream fin(&css_file);
    style = fin.readAll();
}
