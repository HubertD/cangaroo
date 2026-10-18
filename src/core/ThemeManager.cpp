/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QStyle>
#include <QStyleFactory>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), _currentTheme(Light)
{
    updateColors(Light);
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

void ThemeManager::applyTheme(Theme theme)
{
    _currentTheme = theme;
    updateColors(theme);
    applyPalette(theme);
    applyStyleSheet(theme);
    emit themeChanged(theme);
}

void ThemeManager::setCustomTraceTextColor(const QColor &color)
{
    _customTraceTextColor = color;
    emit traceTextColorChanged(color);
}

void ThemeManager::updateColors(Theme theme)
{
    if (theme == Light) {
        // Capture default system colors or standard light theme
        _colors.window = QColor(240, 240, 240);
        _colors.windowText = Qt::black;
        _colors.base = Qt::white;
        _colors.alternateBase = QColor(233, 231, 227);
        _colors.toolTipBase = Qt::white;
        _colors.toolTipText = Qt::black;
        _colors.text = Qt::black;
        _colors.button = QColor(240, 240, 240);
        _colors.buttonText = Qt::black;
        _colors.brightText = Qt::white;
        _colors.link = QColor(0, 0, 255);
        _colors.highlight = QColor(48, 140, 198);
        _colors.highlightedText = Qt::white;
        
        _colors.graphBackground = Qt::white;
        _colors.graphGrid = QColor(230, 230, 230);
        _colors.graphAxisText = Qt::black;
        _colors.graphCursor = Qt::black;
    } else {
        _colors.window = QColor(45, 45, 48);
        _colors.windowText = QColor(220, 220, 220);
        _colors.base = QColor(30, 30, 30);
        _colors.alternateBase = QColor(37, 37, 38);
        _colors.toolTipBase = Qt::white;
        _colors.toolTipText = Qt::black;
        _colors.text = QColor(220, 220, 220);
        _colors.button = QColor(45, 45, 48);
        _colors.buttonText = QColor(220, 220, 220);
        _colors.brightText = Qt::white;
        _colors.link = QColor(86, 156, 214);
        _colors.highlight = QColor(38, 79, 120);
        _colors.highlightedText = Qt::white;
        
        _colors.graphBackground = QColor(25, 25, 25);
        _colors.graphGrid = QColor(60, 60, 60);
        _colors.graphAxisText = QColor(180, 180, 180);
        _colors.graphCursor = Qt::white;
    }
}

void ThemeManager::applyPalette(Theme theme)
{
    if (theme == Light) {
        qApp->setPalette(qApp->style()->standardPalette());
    } else {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, _colors.window);
        darkPalette.setColor(QPalette::WindowText, _colors.windowText);
        darkPalette.setColor(QPalette::Base, _colors.base);
        darkPalette.setColor(QPalette::AlternateBase, _colors.alternateBase);
        darkPalette.setColor(QPalette::ToolTipBase, _colors.toolTipBase);
        darkPalette.setColor(QPalette::ToolTipText, _colors.toolTipText);
        darkPalette.setColor(QPalette::Text, _colors.text);
        darkPalette.setColor(QPalette::Button, _colors.button);
        darkPalette.setColor(QPalette::ButtonText, _colors.buttonText);
        darkPalette.setColor(QPalette::BrightText, _colors.brightText);
        darkPalette.setColor(QPalette::Link, _colors.link);
        darkPalette.setColor(QPalette::Highlight, _colors.highlight);
        darkPalette.setColor(QPalette::HighlightedText, _colors.highlightedText);
        
        // Disabled colors
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
        
        qApp->setPalette(darkPalette);
    }
}

void ThemeManager::applyStyleSheet(Theme theme)
{
    QString qssPath = (theme == Light) ? ":/assets/light_theme.qss" : ":/assets/dark_theme.qss";
    QFile file(qssPath);
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    } else {
        qApp->setStyleSheet("");
    }
}
