/*
    SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidstylepluginiconloader.h"
#include <QAndroidJniObject>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

AndroidStylePluginIconLoader::AndroidStylePluginIconLoader()
{
    m_modifiedIconDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/modifiedIcons"));
    int versionCode = QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getVersionCode");
    const QString versionString = QStringLiteral("%1").arg(versionCode);

    // Invalidate the cache after an application update
    if (!m_modifiedIconDir.exists(versionString)) {
        m_modifiedIconDir.removeRecursively();
        m_modifiedIconDir.mkpath(versionString);
    }
    bool success = m_modifiedIconDir.cd(versionString);
    assert(success);
}

QIcon AndroidStylePluginIconLoader::getIcon(const IconQuery &q)
{
    const QIcon *cachedIcon = m_iconCache.object(q);
    if (cachedIcon != nullptr) {
        return *cachedIcon;
    }

    QIcon loadedIcon = loadIcon(q);
    m_iconCache.insert(q, new QIcon(loadedIcon));
    return loadedIcon;
}

QIcon AndroidStylePluginIconLoader::loadIcon(const IconQuery &q)
{
    const QString basePath = getSvg(q.name, q.baseColor);
    if (basePath.isEmpty()) {
        return QIcon::fromTheme(q.name);
    }
    QIcon result(basePath);

    const QString highlightPath = getSvg(q.name, q.highlightColor);
    if (!highlightPath.isEmpty()) {
        result.addFile(highlightPath, QSize(), QIcon::Selected);
    }

    return result;
}

QString AndroidStylePluginIconLoader::getSvg(const QString &iconName, const QColor &color)
{
    const QString colorString = color.name();
    const QString iconPath = m_modifiedIconDir.absoluteFilePath(QLatin1String("%1/%2.svg").arg(colorString).arg(iconName));
    if (QFileInfo::exists(iconPath)) {
        return iconPath;
    }
    m_modifiedIconDir.mkpath(colorString);

    if (color == Qt::transparent || color == Qt::black) {
        // We don't need any modifications to the source, but copy
        // it anyway so that we only have to hit the find algorithm
        // on first run.
        if (copySvg(iconName, iconPath)) {
            return iconPath;
        }
        return QString();
    }

    const bool didMake = makeSvg(iconPath, iconName, colorString);
    if (didMake) {
        return iconPath;
    }
    return QString();
}

bool AndroidStylePluginIconLoader::makeSvg(const QString &iconPath, const QString &iconName, const QString &colorString)
{
    const QString source = findSourceForName(iconName);
    if (source.isEmpty()) {
        return false;
    }

    // TODO what if an icon uses other theme colors?
    const QString css = QLatin1String(".ColorScheme-Text { color:%1; }").arg(colorString);
    QFile inFile(source);
    QFile outFile(iconPath);
    if (!(inFile.open(QFile::ReadOnly) && outFile.open(QFile::WriteOnly))) {
        qDebug() << "Failed to open icon svg files";
        return false;
    }

    QXmlStreamReader reader(&inFile);
    QXmlStreamWriter writer(&outFile);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.qualifiedName() == QLatin1String("style")
            && reader.attributes().value(QLatin1String("id")) == QLatin1String("current-color-scheme")) {
            writer.writeStartElement(QStringLiteral("style"));
            writer.writeAttributes(reader.attributes());
            writer.writeCharacters(css);
            writer.writeEndElement();
            while (reader.tokenType() != QXmlStreamReader::EndElement) {
                reader.readNext();
            }
        } else if (reader.tokenType() != QXmlStreamReader::Invalid) {
            writer.writeCurrentToken(reader);
        } else {
            qDebug() << "abandoning svg coloring because of an invalid token";
            return false;
        }
    }

    return true;
}

bool AndroidStylePluginIconLoader::copySvg(const QString &iconName, const QString &destPath)
{
    const QString sourcePath = findSourceForName(iconName);
    if (sourcePath.isEmpty()) {
        return false;
    }
    return QFile::copy(sourcePath, destPath);
}

QString AndroidStylePluginIconLoader::findSourceForName(const QString &iconName)
{
    // did we already find this before?
    if (m_iconNameIndex.contains(iconName)) {
        return m_iconNameIndex[iconName];
    }
    qDebug() << "searching for icon (slow):" << iconName;
    QDir curDir;
    QString findName = QLatin1String("%1.svg").arg(iconName);
    QString outName;
    for (const QString &path : QIcon::themeSearchPaths()) {
        curDir.setPath(path);
        if (findSourceForName(iconName, findName, curDir, outName)) {
            break;
        }
    }
    return outName;
}

bool AndroidStylePluginIconLoader::findSourceForName(const QString &iconName, const QString &findName, QDir &curDir, QString &outName)
{
    // this does not implement anything like the XDG icon theme spec,
    // but it's close enough for our purposes.  It's slow, but we should
    // only hit it on the first run unless there are icons missing from
    // the package.
    for (const QFileInfo &candidate : curDir.entryInfoList()) {
        if (candidate.fileName() == findName) {
            outName = candidate.absoluteFilePath();
            m_iconNameIndex.insert(iconName, outName);
            return true;
        }

        if (candidate.isDir()) {
            curDir.cd(candidate.fileName());
            if (findSourceForName(iconName, findName, curDir, outName)) {
                return true;
            }
            curDir.cd("..");
        } else if (candidate.suffix() == "svg") {
            // this is not the one we're looking for, but remember it in case
            // we need to look it up later
            m_iconNameIndex.insert(candidate.baseName(), candidate.absoluteFilePath());
        }
    }
    return false;
}
