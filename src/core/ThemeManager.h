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

#pragma once

#include <QObject>
#include <QColor>
#include <QPalette>
#include <QString>

struct ThemeColors {
    QColor window;
    QColor windowText;
    QColor base;
    QColor alternateBase;
    QColor toolTipBase;
    QColor toolTipText;
    QColor text;
    QColor button;
    QColor buttonText;
    QColor brightText;
    QColor link;
    QColor highlight;
    QColor highlightedText;
    
    // Graph specific
    QColor graphBackground;
    QColor graphGrid;
    QColor graphAxisText;
    QColor graphCursor;
};

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum Theme {
        Light,
        Dark
    };

    static ThemeManager& instance();

    void applyTheme(Theme theme);
    Theme currentTheme() const { return _currentTheme; }
    bool isDarkMode() const { return _currentTheme == Dark; }
    
    const ThemeColors& colors() const { return _colors; }

    QColor customTraceTextColor() const { return _customTraceTextColor; }
    void setCustomTraceTextColor(const QColor &color);

signals:
    void themeChanged(Theme theme);
    void traceTextColorChanged(QColor color);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    Theme _currentTheme;
    ThemeColors _colors;
    QColor _customTraceTextColor;
    
    void updateColors(Theme theme);
    void applyStyleSheet(Theme theme);
    void applyPalette(Theme theme);
};
