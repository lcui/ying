#ifndef __GIT_ENGINE__H
#define __GIT_ENGINE__H
#include <QString>
#include <QObject>

struct Commit : public QObject
{
public:
    Commit(QObject* parent):QObject(parent){};
    QString id;
    QString author;
    QString date;
    QString comment;
};

class QProcess;
class GitEngine: public QObject
{
public:
    GitEngine(QObject* parent);

public:
    QString     getEngine();
    bool        setEngine(const QString& path);
    QStringList getFileLog(const QString& file, const char* flags);
    QList<Commit*> getFileLog(const QString& file);
    QStringList getFileCommitList(const QString& file, const char* flags);
    QStringList getCommitFileList(const QString& commit, const char* flags);/* commit is empty means TOT*/
    QStringList getCommitBranchList(const QString& commit);
    QStringList getCommitTagList(const QString& commit);
    QStringList getFileContent(const QString& file, const QString& commit);
    QStringList getCommitContent(const QString& commit, const QString& file="");
    QString     getRootDir(const QString& file);
    QStringList getWorkspaceFiles(const QString& file, const QString& commit);
    QStringList getCommitsDiff(const QString& file, const QString& commit1, const QString& commit2);

private:
    QByteArray run(const QString& cmd);
private:
    QObject *mpParent;
    QString mGitPath;
    QProcess* mpProc;
};
#endif /*__GIT_ENGINE__H*/
