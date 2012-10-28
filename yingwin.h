#ifndef __YING_WIN__H
#define __YING_WIN__H
#include <QString>
#include <QMainWindow>
#include <QMap>

class GitEngine;
class QTextBrowser;
class QTreeWidgetItem;
class QListWidgetItem;
class QLineEdit;
class YingWin : public QMainWindow
{
    Q_OBJECT
public:
    YingWin(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~YingWin();

public:
    void analyzeFile(const QString& file);

protected slots:
    void onTreeItemClicked (QTreeWidgetItem *item, int column);
    void onCmtsTreeItemClicked(QTreeWidgetItem *item, int column);
    void onNewFileFilter();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    void fillWorkspace(const QStringList& files, const QString& commit, const QString& name);


private:
    GitEngine       *mpEngine;
    QTextBrowser    *mpBrowser;
    QLineEdit       *mpFileFilter;
    QMap<QString, QWidget*> mMapInfoWin;
    QString         mCurrRoot;
    QString         mCurrFile;
    QStringList     mCommitList;    /* Commit ID list for mCurrFile */
};
#endif /*__YING_WIN__H*/
