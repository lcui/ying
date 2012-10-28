#include "yingwin.h"
#include "gitengine.h"
#include "txt2html.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QAction>
#include <QLayout>
#include <QFormLayout>
#include <QMenu>
#include <QTextEdit>
#include <QMessageBox>
#include <QTextBrowser>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <qdebug.h>
#include <QTimer>
#include <QDate>
#include <QDockWidget>
#include <QSplitter>
#include <QListWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <cassert>

const char compName[] = "Daydayup Inc.";
const char appName[] = "Ying";
YingWin::YingWin(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setWindowIcon(QIcon(":/images/ying.ico"));

    mpEngine = new GitEngine(this);
    QString gitPath = mpEngine->getEngine();

    if (gitPath.isEmpty()) {
        QSettings settings(compName, appName);
        gitPath = settings.value("GitPath").toString();
        if (gitPath.isEmpty()) {
            int ret = QMessageBox::warning(this, "Warning", 
                    tr("git path is not set yet, set it now?"), 
                    QMessageBox::No|QMessageBox::Yes);
            if (QMessageBox::Yes == ret) {
                gitPath = QFileDialog::getOpenFileName(this,
                        tr("Open file"), 
                        ".",    /* path */
                        tr("All(*.*)"));
            }

            settings.setValue("GitPath", gitPath);
        }
        /*FIXME*/
        gitPath = QDir::toNativeSeparators("bin/git.exe");
        mpEngine->setEngine(gitPath);
    }

    QSplitter *splitter = new QSplitter(this);
    QTreeWidget *pTreeHdrs = new QTreeWidget(splitter);
    pTreeHdrs->setColumnCount(1);
    pTreeHdrs->setHeaderHidden(true);
    connect(pTreeHdrs, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
            this, SLOT(onTreeItemClicked(QTreeWidgetItem*, int)));
    mMapInfoWin.insert("FileList", pTreeHdrs);
    splitter->addWidget(pTreeHdrs);


    QSplitter *right = new QSplitter(Qt::Vertical, splitter);

    QSplitter *commit= new QSplitter(right);
    QTreeWidget *tree = new QTreeWidget(commit);
    tree->setColumnCount(3);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(tree,SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
            this, SLOT(onCmtsTreeItemClicked(QTreeWidgetItem*, int)));

    mMapInfoWin.insert("CommitList", tree);
    commit->addWidget(tree);

    QListWidget *list = new QListWidget(right);
    commit->addWidget(list);
    mMapInfoWin.insert("CommitFiles", list);
    commit->setStretchFactor(0, 10);

    right->addWidget(commit);

    mpBrowser = new QTextBrowser(right);
    right->addWidget(mpBrowser);
    right->setStretchFactor(2, 10);

    splitter->addWidget(right);
    splitter->setStretchFactor(1, 10);
    setCentralWidget(splitter);
}

YingWin::~YingWin()
{
    if (mpEngine) {
        delete mpEngine;
        mpEngine = NULL;
    }
}

void 
YingWin::onTreeItemClicked (QTreeWidgetItem *item, int column)
{
    analyzeFile(mCurrRoot+"/"+item->toolTip(0));
}

void 
YingWin::onCmtsTreeItemClicked(QTreeWidgetItem *curr, int column)
{
    if (curr) {
        QTreeWidget* tree = qobject_cast<QTreeWidget*>(mMapInfoWin["CommitList"]);
        QList<QTreeWidgetItem *> tlst = tree->selectedItems();
        //printf("tlst->length()=%d\n", tlst.length());
        YTxt2Html txt2html;
        txt2html.prepare();
        if (tlst.length() == 1) {
            QString commit = curr->toolTip(0);
            QStringList files = mpEngine->getCommitFileList(commit, NULL);
            QListWidget *fileList = qobject_cast<QListWidget*>(mMapInfoWin["CommitFiles"]);
            fileList->clear();
            for(int i=0; i<files.length(); i++) {
                fileList->addItem(files[i]);
            }

            QStringList content = mpEngine->getCommitContent(commit);
            for(int i=0; i<content.length(); i++) {
                txt2html.append((content[i]+'\n').toStdString());
            }
        } else if (tlst.length() == 2) {
            QString commit1 = tlst[0]->toolTip(0);
            QString commit2 = tlst[1]->toolTip(0);

            QStringList content = mpEngine->getCommitsDiff(mCurrFile, commit1, commit2);
            for(int i=0; i<content.length(); i++) {
                txt2html.append((content[i]+'\n').toStdString());
            }
        } else {
            /* remove others */
        }

        mpBrowser->setHtml(txt2html.finish().c_str());
    } else {
        /* FIXME: choose the 1st item? */
    }
}

void 
YingWin::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);
    
    // accept just text/uri-list mime format
    if (event->mimeData()->hasFormat("text/uri-list")) 
    {     
        event->acceptProposedAction();
    }
}

void 
YingWin::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;
 
    setBackgroundRole(QPalette::Window);
    if (event->mimeData()->hasUrls())
    {
        urlList = event->mimeData()->urls(); // returns list of QUrls
    
        // if just text was dropped, urlList is empty (size == 0)
        if ( urlList.size() > 0) // if at least one QUrl is present in list
        {
            fName = urlList[0].toLocalFile(); // convert first QUrl to local path
            info.setFile( fName ); // information about file
            if ( info.isFile() ) {
                analyzeFile(fName);
            }
        }
    }
    
    event->acceptProposedAction();
}

void
YingWin::fillWorkspace(const QStringList& files, const QString& commit, const QString& name)
{
    QTreeWidget* tree = qobject_cast<QTreeWidget*>(mMapInfoWin["FileList"]);

    setUpdatesEnabled(false);
    tree->clear();
    QVector<QTreeWidgetItem *>branches(300);
    QTreeWidgetItem *root = new QTreeWidgetItem(tree);
    root->setText(0, name);
    branches[0] = root;

    /* assume the result is sorted */
    for(int i=0; i<files.length(); i++) {
        const QString &t = files[i];
        if (t.isEmpty()) {
            continue;
        }
        QStringList dirs = t.split("/");
        for(int j=0; j<dirs.length(); j++) {
            const QString &d = dirs[j];
            if (!branches[j+1] || branches[j+1]->text(0) != d){
                QTreeWidgetItem *node = new QTreeWidgetItem(branches[j]);
                node->setText(0, d);
                node->setToolTip(0, t);
                branches[j+1] = node;
            }
        }
    }
    setUpdatesEnabled(true);
}

void 
YingWin::analyzeFile(const QString& file)
{
    /* Update Commit ID window */
    mCurrFile = file;
    QString root = mpEngine->getRootDir(mCurrFile);
    if (mCurrRoot != root) {
        QStringList files = mpEngine->getWorkspaceFiles(file, "HEAD");
        fillWorkspace(files, "HEAD", root);
        mCurrRoot = root;
    }
    QList<Commit*> commits = mpEngine->getFileLog(mCurrFile);
    if (commits.length()) {
        QTreeWidget* tree = qobject_cast<QTreeWidget*>(mMapInfoWin["CommitList"]);
        setUpdatesEnabled(false);
        tree->clear();
        QTreeWidgetItem *header = new QTreeWidgetItem();
        header->setText(0, "Change");
        header->setText(1, "Author");
        header->setText(2, "Date");
        tree->setHeaderItem(header);
        tree->header()->resizeSection(0, 600);

        for(int i=0; i<commits.length(); i++) {
            QTreeWidgetItem *item = new QTreeWidgetItem(tree);
            item->setText(0, commits[i]->comment.split("\n")[0]);
            item->setText(1, commits[i]->author);
            item->setText(2, commits[i]->date);
            item->setToolTip(0, commits[i]->id);
        }
        setUpdatesEnabled(true);
        mCommitList = mpEngine->getFileCommitList(mCurrFile, NULL);
    }
}

//#include "yingwin.moc"

