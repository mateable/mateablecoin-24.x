// Copyright (c) 2015-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/platformstyle.h>

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QPalette>
#include <QSettings>

static const struct {
    const char *platformId;
    /** Show images on push buttons */
    const bool imagesOnButtons;
    /** Colorize single-color icons */
    const bool colorizeIcons;
    /** Extra padding/spacing in transactionview */
    const bool useExtraSpacing;
} platform_styles[] = {
    {"macosx", false, true, true},
    {"windows", true, true, false},
    /* Other: linux, unix, ... */
    {"other", true, true, false}
};

namespace {
/* Local functions for colorizing single-color images */

void MakeSingleColorImage(QImage& img, const QColor& colorbase)
{
    img = img.convertToFormat(QImage::Format_ARGB32);
    for (int x = img.width(); x--; )
    {
        for (int y = img.height(); y--; )
        {
            const QRgb rgb = img.pixel(x, y);
            img.setPixel(x, y, qRgba(colorbase.red(), colorbase.green(), colorbase.blue(), qAlpha(rgb)));
        }
    }
}

QIcon ColorizeIcon(const QIcon& ico, const QColor& colorbase)
{
    QIcon new_ico;
    for (const QSize& sz : ico.availableSizes())
    {
        QImage img(ico.pixmap(sz).toImage());
        MakeSingleColorImage(img, colorbase);
        new_ico.addPixmap(QPixmap::fromImage(img));
    }
    return new_ico;
}

QImage ColorizeImage(const QString& filename, const QColor& colorbase)
{
    QImage img(filename);
    MakeSingleColorImage(img, colorbase);
    return img;
}

QIcon ColorizeIcon(const QString& filename, const QColor& colorbase)
{
    return QIcon(QPixmap::fromImage(ColorizeImage(filename, colorbase)));
}

}


PlatformStyle::PlatformStyle(const QString &_name, bool _imagesOnButtons, bool _colorizeIcons, bool _useExtraSpacing):
    name(_name),
    imagesOnButtons(_imagesOnButtons),
    colorizeIcons(_colorizeIcons),
    useExtraSpacing(_useExtraSpacing)
{
    QSettings settings;
    darkTheme = settings.value("fDarkTheme", true).toBool();
}

QColor PlatformStyle::TextColor() const
{
    return QApplication::palette().color(QPalette::WindowText);
}

QColor PlatformStyle::SingleColor() const
{
    // Return theme accent color for icon colorization
    return QColor(0xcc, 0xcc, 0xcc); // light grey — visible on dark background
}

QImage PlatformStyle::SingleColorImage(const QString& filename) const
{
    if (!darkTheme) {
        QString lightFile = QString(filename).replace(":/icons/", ":/icons-light/");
        if (QFile::exists(lightFile))
            return QImage(lightFile);
    }
    return QImage(filename);
}

QIcon PlatformStyle::SingleColorIcon(const QString& filename) const
{
    if (!darkTheme) {
        QString lightFile = QString(filename).replace(":/icons/", ":/icons-light/");
        if (QFile::exists(lightFile))
            return QIcon(lightFile);
    }
    return QIcon(filename);
}

QIcon PlatformStyle::SingleColorIcon(const QIcon& icon) const
{
    return icon;
}

QIcon PlatformStyle::TextColorIcon(const QIcon& icon) const
{
    return icon;
}

const PlatformStyle *PlatformStyle::instantiate(const QString &platformId)
{
    for (const auto& platform_style : platform_styles) {
        if (platformId == platform_style.platformId) {
            return new PlatformStyle(
                    platform_style.platformId,
                    platform_style.imagesOnButtons,
                    platform_style.colorizeIcons,
                    platform_style.useExtraSpacing);
        }
    }
    return nullptr;
}

