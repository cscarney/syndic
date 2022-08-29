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
    QString iconPath = m_modifiedIconDir.absoluteFilePath(QLatin1String("%1/%2.svg").arg(colorString).arg(iconName));
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

static bool fallbackName(QString &name)
{
    int splitPos = name.lastIndexOf('-');
    if (splitPos <= 0) {
        return false;
    }
    name = name.mid(0, splitPos);
    return true;
}

QString AndroidStylePluginIconLoader::findSourceForName(const QString &iconName)
{
    if (m_iconNameIndex.isEmpty()) {
        indexIconTheme();
    }

    QString namePart{iconName};
    do {
        if (m_iconNameIndex.contains(namePart)) {
            return m_iconNameIndex[namePart];
        }
    } while (fallbackName(namePart));
    return QString();
}

void AndroidStylePluginIconLoader::indexIconTheme()
{
    QDir curDir;
    const QStringList themeSearchPaths = QIcon::themeSearchPaths();
    for (const QString &path : themeSearchPaths) {
        curDir.setPath(path);
        indexIconDir(curDir);
    }
}

void AndroidStylePluginIconLoader::indexIconDir(QDir &curDir)
{
    const QFileInfoList dirEntries = curDir.entryInfoList();
    for (const QFileInfo &candidate : dirEntries) {
        if (candidate.isDir()) {
            curDir.cd(candidate.fileName());
            indexIconDir(curDir);
            curDir.cd("..");
        } else if (candidate.suffix() == "svg") {
            m_iconNameIndex.insert(candidate.baseName(), candidate.absoluteFilePath());
        }
    }
}
