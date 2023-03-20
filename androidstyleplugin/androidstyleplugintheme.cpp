/*
    SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidstyleplugintheme.h"
#include "androidstylepluginiconloader.h"
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QCache>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>

namespace
{

struct StyleData {
    // from https://developer.android.com/reference/android/R.attr
    enum AndroidResourceId {
        COLOR_ACCENT = 16843829,
        COLOR_BACKGROUND = 16842801,
        COLOR_BACKGROUND_FLOATING = 16844002,
        COLOR_FOREGROUND = 16842800,
        COLOR_FOREGROUND_INVERSE = 16843270,
        TEXT_COLOR_LINK = 16842907,
    };

    QPalette basePalette;
    QPalette inversePalette;
    QColor accentColor;

    QFont smallFont;

    AndroidStylePluginIconLoader iconLoader;
    QObject *materialHelper{nullptr};

    static StyleData *createIfNecessary(QQmlEngine *engine = nullptr);
    void loadStyle();

private:
    StyleData(QQmlEngine *engine);
    friend StyleData *styleData();
};

StyleData *styleData()
{
    return StyleData::createIfNecessary();
}

constexpr const int kDarkModeValueThreshold = 128;
constexpr const float kUnscaledDefaultFontSize = 16;
constexpr const float kUnscaledSmallFontSize = 14;
}

extern "C" {
// TODO remove this once we can handle activity recreation
void syndic_android_refreshStyle(JNIEnv *, jobject)
{
    AndroidStylePluginTheme::refreshInstances();
}
}

StyleData::StyleData(QQmlEngine *engine)
{
    QQmlComponent materialHelperComponent(engine, QUrl("qrc:/qml/materialhelper.qml"));
    materialHelper = materialHelperComponent.create();
    loadStyle();

    QStringList themePaths = QIcon::themeSearchPaths();
    themePaths.append("assets:/share/icons");
    QIcon::setThemeSearchPaths(themePaths);

    QAndroidJniEnvironment env;
    JNINativeMethod nativeMethod = {"refreshStyle", "()V", (void*)syndic_android_refreshStyle};
    jclass c = env->FindClass("com/rocksandpaper/syndic/SyndicActivity");
    env->RegisterNatives(c, &nativeMethod, 1);
}

void StyleData::loadStyle()
{
    accentColor = QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", COLOR_ACCENT);
    QRgb floatingBackgroundColor =
        QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", COLOR_BACKGROUND_FLOATING);
    QRgb backgroundColor = QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", COLOR_BACKGROUND);
    QRgb foregroundInverseColor =
        QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", COLOR_FOREGROUND_INVERSE);
    QRgb textColor = QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", COLOR_FOREGROUND);
    QRgb linkColor = QAndroidJniObject::callStaticMethod<jint>("com/rocksandpaper/syndic/NativeHelper", "getColor", "(I)I", TEXT_COLOR_LINK);

    QColor disabledTextColor = textColor;
    disabledTextColor.setAlphaF(0.8); // TODO theme value for this?

    basePalette = qGuiApp->palette();
    basePalette.setColor(QPalette::Window, backgroundColor);
    basePalette.setColor(QPalette::WindowText, textColor);
    basePalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledTextColor);
    basePalette.setColor(QPalette::Highlight, accentColor);
    basePalette.setColor(QPalette::HighlightedText, foregroundInverseColor);
    basePalette.setColor(QPalette::Base, foregroundInverseColor);
    basePalette.setColor(QPalette::AlternateBase, floatingBackgroundColor);
    basePalette.setColor(QPalette::Text, textColor);
    basePalette.setColor(QPalette::Link, linkColor);
    basePalette.setColor(QPalette::LinkVisited, linkColor);
    qGuiApp->setPalette(basePalette);

    auto fontScale = QAndroidJniObject::callStaticMethod<jfloat>("com/rocksandpaper/syndic/NativeHelper", "getFontScale");
    QFont font = qGuiApp->font();
    font.setPointSizeF(kUnscaledDefaultFontSize * fontScale);
    font.resolve(QFont::AllPropertiesResolved);
    qApp->setFont(font);
    font.setPointSizeF(kUnscaledSmallFontSize * fontScale);
    smallFont = font;
}

StyleData *StyleData::createIfNecessary(QQmlEngine *engine)
{
    static auto *styleData = new StyleData(engine);
    return styleData;
}

static QSet<AndroidStylePluginTheme *> themeInstances;

AndroidStylePluginTheme::AndroidStylePluginTheme(QObject *parent)
    : Kirigami::PlatformTheme(parent)
{
    Q_INIT_RESOURCE(androidstyleplugin);
    themeInstances.insert(this);
    StyleData::createIfNecessary(qmlEngine(parent));
    refresh();
    setSupportsIconColoring(true);
}

AndroidStylePluginTheme::~AndroidStylePluginTheme()
{
    themeInstances.remove(this);
}

QIcon AndroidStylePluginTheme::iconFromTheme(const QString &name, const QColor &customColor)
{
    if (customColor == Qt::transparent) {
        return styleData()->iconLoader.getIcon({name, textColor().rgba(), highlightedTextColor().rgba()});
    }
    return styleData()->iconLoader.getIcon({name, customColor.rgba(), customColor.rgba()});
}

void AndroidStylePluginTheme::refreshInstances()
{
    styleData()->loadStyle();
    for(auto *instance : std::as_const(themeInstances)) {
        instance->refresh();
    }
}

bool AndroidStylePluginTheme::event(QEvent *event)
{
    if (event->type() == Kirigami::PlatformThemeEvents::ColorSetChangedEvent::type) {
        updateColors();
    }
    if (event->type() == Kirigami::PlatformThemeEvents::ColorGroupChangedEvent::type) {
        updateColors();
    }
    return PlatformTheme::event(event);
}

void AndroidStylePluginTheme::refresh()
{
    updateColors();
    setSmallFont(styleData()->smallFont);
    setDefaultFont(qGuiApp->font());
}

void AndroidStylePluginTheme::updateColors()
{
    const StyleData *style = styleData();
    QPalette pal = style->basePalette;

    switch (colorSet()) {
    default:
    case Window:
        setTextColor(pal.color(QPalette::WindowText));
        setDisabledTextColor(pal.color(QPalette::Disabled, QPalette::WindowText));
        setHighlightedTextColor(pal.color(QPalette::HighlightedText));
        setActiveTextColor(pal.color(QPalette::WindowText));
        setLinkColor(pal.color(QPalette::Link));
        setVisitedLinkColor(pal.color(QPalette::LinkVisited));
        setNegativeTextColor(pal.color(QPalette::WindowText));
        setNeutralTextColor(pal.color(QPalette::WindowText));
        setPositiveTextColor(pal.color(QPalette::WindowText));

        setBackgroundColor(pal.color(QPalette::Window));
        setAlternateBackgroundColor(pal.color(QPalette::AlternateBase));
        setHighlightColor(pal.color(QPalette::Highlight));
        setActiveBackgroundColor(pal.color(QPalette::Window));
        setLinkBackgroundColor(pal.color(QPalette::Window));
        setVisitedLinkBackgroundColor(pal.color(QPalette::Window));
        setNegativeBackgroundColor(pal.color(QPalette::Window));
        setNeutralBackgroundColor(pal.color(QPalette::Window));
        setPositiveBackgroundColor(pal.color(QPalette::Window));

        setHoverColor(style->accentColor);
        setFocusColor(style->accentColor);
        break;

    case View:
        setTextColor(pal.color(QPalette::Text));
        setDisabledTextColor(pal.color(QPalette::Disabled, QPalette::Base));
        setHighlightedTextColor(pal.color(QPalette::HighlightedText));
        setActiveTextColor(pal.color(QPalette::Text));
        setLinkColor(pal.color(QPalette::Link));
        setVisitedLinkColor(pal.color(QPalette::LinkVisited));
        setNegativeTextColor(pal.color(QPalette::Text));
        setNeutralTextColor(pal.color(QPalette::Text));
        setPositiveTextColor(pal.color(QPalette::Text));

        setBackgroundColor(pal.color(QPalette::Base));
        setAlternateBackgroundColor(pal.color(QPalette::AlternateBase));
        setHighlightColor(pal.color(QPalette::Highlight));
        setActiveBackgroundColor(pal.color(QPalette::Base));
        setLinkBackgroundColor(pal.color(QPalette::Base));
        setVisitedLinkBackgroundColor(pal.color(QPalette::Base));
        setNegativeBackgroundColor(pal.color(QPalette::Base));
        setNeutralBackgroundColor(pal.color(QPalette::Base));
        setPositiveBackgroundColor(pal.color(QPalette::Base));

        setHoverColor(style->accentColor);
        setFocusColor(style->accentColor);
        break;
    }

    // the material theme settings are private in CPP but exposed in qml
    // so we call out to a qml helper object.
    auto *item = parent();
    auto *helper = styleData()->materialHelper;
    bool isDarkMode = backgroundColor().value() < kDarkModeValueThreshold;
    QMetaObject::invokeMethod(helper,
                              "updateMaterialTheme",
                              Q_ARG(QVariant, QVariant::fromValue(item)),
                              Q_ARG(QVariant, QVariant::fromValue(isDarkMode)),
                              Q_ARG(QVariant, QVariant::fromValue(textColor())),
                              Q_ARG(QVariant, QVariant::fromValue(backgroundColor())),
                              Q_ARG(QVariant, QVariant::fromValue(highlightColor())),
                              Q_ARG(QVariant, QVariant::fromValue(highlightColor())));
}
