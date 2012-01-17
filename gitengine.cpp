#include "gitengine.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <cassert>

const char gitcmd[] = "c:\\program files(x86)\\Git\\bin\\git.exe";

GitEngine::GitEngine(QObject* parent)
    :   QObject(parent) 
      , mpParent(parent)
      , mGitPath("git")
{
    mpProc = new QProcess;
}

QList<Commit*>
GitEngine::getFileLog(const QString& file)
{
    QList<Commit*> commits;
    QStringList list = getFileLog(file, NULL);
    for(int i=0; i<list.length(); i++) {
        QStringList cmt = list[i].split("\n");
        Commit *one = new Commit(this);
        for(int j=0; j<cmt.length(); j++) {
            QString str = cmt[j];
            if (str.isEmpty()) {
                continue;
            }
            if (str.startsWith("commit ")) {
                one->id = str.split(" ")[1];
            } else if (str.startsWith("Author:")) {
                one->author = str.right(str.length()-7);
            } else if (str.startsWith("Date:")) {
                one->date = str.right(str.length()-5);
            } else {
                one->comment += str+"\n";
            }
        }
        commits.append(one);
    }

    return commits;
}

QStringList 
GitEngine::getFileLog(const QString& file, const char* flags)
{
    QString cmd = mGitPath + " log " + file;
    QByteArray output = run(cmd);

    QList<QByteArray> slist = output.split('\n');
    QStringList retlist;

    QString oneCommit;
    for(int i=0; i<slist.length(); i++) {
        if(slist[i].startsWith("commit ")) {
            if (!oneCommit.isEmpty()) {
                retlist << oneCommit;
            }
            oneCommit = slist[i] + "\n";
        } else {
            oneCommit += slist[i] + "\n";
        }
    }
    if (!oneCommit.isEmpty()) {
        retlist << oneCommit;
    }

    return retlist;
}

QByteArray 
GitEngine::run(const QString& cmd)
{
    mpProc->start(cmd);
    if (mpProc->waitForStarted()) {
        mpProc->waitForFinished();
    } else {
        QProcess::ProcessError err = mpProc->error();
        printf("error = %d\n", err);
    }

    QByteArray output = mpProc->readAllStandardOutput();

#if defined(_DEBUG) || defined(DEBUG)
    {
        FILE* fout = fopen("debug.log", "wb");
        fwrite(output.data(), 1, output.length(), fout);
        fclose(fout);
    }
#endif

    return output;
}

QStringList 
GitEngine::getFileCommitList(const QString& file, const char* flags)
{
    QString cmd = mGitPath + " log --pretty=oneline " + file;

    QByteArray output = run(cmd);

    QList<QByteArray> slist = output.split('\n');
    QStringList retlist;

    for(int i=0; i<slist.length(); i++) {
        if (slist[i].length()) {
            QString line = slist[i];
            retlist << line.split(" ")[0];
        }
    }

    return retlist;
}

QStringList
GitEngine::getFileContent(const QString& file, const QString& commit)
{
    QString root = getRootDir(file);
    QString nf = file.right(file.length()-root.length());
    QString cmd = mGitPath + QObject::tr(" show  %1:%2").arg(commit).arg(nf);
    QList<QByteArray> slist = run(cmd).split('\n');
    QStringList retlist;
    for(int i=0; i<slist.length(); i++) {
        if (slist[i].length()) {
            retlist << slist[i];
        }
    }

    return retlist;
}

QString
GitEngine::getRootDir(const QString& file)
{
    QFileInfo gitInfo(file);
    if (!gitInfo.exists()) {
        assert(!"Why here?");
    }
    QString path;
    if (gitInfo.isFile()) {
        path = QDir::toNativeSeparators(gitInfo.absolutePath());
    } else {
        path = QDir::toNativeSeparators(file);
    }
    mpProc->setWorkingDirectory(path);

    QString cmd = mGitPath + " rev-parse --show-toplevel";
    QString output = run(cmd);
    return output.simplified();
}

/* "file" should not be the root directory */
QStringList 
GitEngine::getWorkspaceFiles(const QString& file, const QString& commit)
{
    /* Set workspace first */
    getRootDir(file);

    QString cmd = mGitPath + QObject::tr(" ls-tree -r --full-tree --name-only %1").arg(commit);

    QByteArray output = run(cmd);

    QList<QByteArray> slist = output.split('\n');
    QStringList retlist;

    for(int i=0; i<slist.length(); i++) {
        if (slist[i].length()) {
            retlist << slist[i];
        }
    }

    return retlist;
}

QStringList 
GitEngine::getCommitFileList(const QString& commit, const char* flags)
{
    QString cmd = mGitPath + QObject::tr(" show --pretty=format: --name-only %1").arg(commit);
    QByteArray output = run(cmd);
    QList<QByteArray> slist = output.split('\n');

    QStringList retlist;

    for(int i=0; i<slist.length(); i++) {
        if (slist[i].length()) {
            retlist << slist[i];
        }
    }

    return retlist;
}

QStringList 
GitEngine::getCommitContent(const QString& commit)
{
    QString cmd = mGitPath + QObject::tr(" show %1").arg(commit);
    QByteArray output = run(cmd);
    QList<QByteArray> slist = output.split('\n');

    QStringList retlist;

    for(int i=0; i<slist.length(); i++) {
        if (slist[i].length()) {
            retlist << slist[i];
        }
    }

    return retlist;
}

QString
GitEngine::getEngine()
{
    QString cmd = mGitPath + " version";
    QByteArray output = run(cmd);

    if (output.startsWith("git version")) {
        return mGitPath;
    } else {
        return "";
    }
}

bool
GitEngine::setEngine(const QString& path)
{
    mGitPath = path;
    /* FIXME: check path is valid or not */
    if (getEngine() == path) {
        return true;
    } else {
        return false;
    }
}
