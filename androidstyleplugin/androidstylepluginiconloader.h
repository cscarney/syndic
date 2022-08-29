/*
    SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDSTYLEPLUGINICONLOADER_H
#define ANDROIDSTYLEPLUGINICONLOADER_H
#include <QCache>
#include <QColor>
#include <QDir>
#include <QIcon>

class AndroidStylePluginIconLoader
{
public:
    AndroidStylePluginIconLoader();

    struct IconQuery {
        QString name;
        QRgb baseColor;
        QRgb highlightColor;

        bool operator==(const IconQuery &other) const
        {
            return name == other.name && baseColor == other.baseColor;
        }
    };

    QIcon getIcon(const IconQuery &q);

private:
    QDir m_modifiedIconDir;
    QCache<IconQuery, QIcon> m_iconCache;
    QHash<QString, QString> m_iconNameIndex;

    QIcon loadIcon(const IconQuery &q);
    QString getSvg(const QString &iconName, const QColor &color);
    bool makeSvg(const QString &iconPath, const QString &iconName, const QString &colorString);
    bool copySvg(const QString &iconName, const QString &destPath);
    QString findSourceForName(const QString &iconName);

    void indexIconTheme();
    void indexIconDir(QDir &curDir);
};

inline uint qHash(const AndroidStylePluginIconLoader::IconQuery &q)
{
    return qHash(q.name, qHash(q.baseColor, qHash(q.highlightColor)));
}

#endif // ANDROIDSTYLEPLUGINICONLOADER_H
