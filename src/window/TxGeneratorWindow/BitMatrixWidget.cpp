#include "BitMatrixWidget.h"
#include <QToolTip>
#include <QHelpEvent>

BitMatrixWidget::BitMatrixWidget(QWidget *parent)
    : QWidget(parent), _msg(nullptr), _cellSize(50), _compactMode(false)
{
    _signalColors << QColor("#3498db") << QColor("#e74c3c") << QColor("#2ecc71")
                  << QColor("#f1c40f") << QColor("#9b59b6") << QColor("#1abc9c")
                  << QColor("#e67e22") << QColor("#34495e");
    
    setMouseTracking(true);
}

void BitMatrixWidget::setMessage(CanDbMessage *msg)
{
    _msg = msg;
    updateGeometry();
    update();
}

void BitMatrixWidget::setCellSize(int px)
{
    _cellSize = px;
    updateGeometry();
    update();
}

void BitMatrixWidget::setCompactMode(bool compact)
{
    _compactMode = compact;
    updateGeometry();
    update();
}

void BitMatrixWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int byteCount = (_msg ? (_msg->getDlc() > 8 ? _msg->getDlc() : 8) : 8);
    if (byteCount > 64) byteCount = 64; 

    int cellSize = _cellSize;
    int labelSize = 40;
    int margin = 10;

    int startX = margin + labelSize;
    int startY = margin + labelSize;

    // Draw Column Labels (Bits 7..0) - Top Header
    painter.setPen(palette().color(QPalette::WindowText));
    QFont headerFont = painter.font();
    headerFont.setBold(true);
    painter.setFont(headerFont);
    
    for (int col = 0; col < 8; ++col) {
        QRect labelRect(startX + col * cellSize, margin, cellSize, labelSize);
        painter.drawText(labelRect, Qt::AlignCenter, QString::number(7 - col));
    }

    // Draw Row Labels (Bytes 0..N) - Left Column
    for (int row = 0; row < byteCount; ++row) {
        QRect labelRect(margin, startY + row * cellSize, labelSize, cellSize);
        painter.drawText(labelRect, Qt::AlignCenter, QString::number(row));
    }

    // =========================================================
    // CONFIGURATION: WIDER CELLS
    // =========================================================
    int cellHeight = cellSize;
    int cellWidth = int(cellHeight * 1.8); // Make width 1.8x the height

    // LAYER 1: BACKGROUND GRID
    QColor gridColor = palette().color(QPalette::WindowText);
    gridColor.setAlpha(40);
    painter.setPen(QPen(gridColor, 1));
    painter.setBrush(Qt::NoBrush);
    
    for (int row = 0; row < byteCount; ++row) {
        for (int col = 0; col < 8; ++col) {
            QRect r(startX + col * cellWidth, startY + row * cellHeight, cellWidth, cellHeight);
            painter.drawRect(r); 
        }
    }

    // =========================================================
    // LAYER 2: MERGED SIGNALS (Colored Bars + Text)
    // =========================================================
    if (_msg) {
        // Setup Font: Dynamic size based on cell height (45% of height)
        QFont f = painter.font();
        f.setPixelSize(qMax(10, int(cellHeight * 0.45))); 
        f.setBold(true);
        painter.setFont(f);

        foreach (CanDbSignal *signal, _msg->getSignals()) {
            
            for (int row = 0; row < byteCount; ++row) {
                
                // --- STEP A: CALCULATE VISUAL SPAN ---
                int visualLeft = -1;  // Bit 7 side
                int visualRight = 99; // Bit 0 side
                bool hasSignal = false;

                for (int b = 7; b >= 0; --b) {
                    if (getBitInfo(row, b).signal == signal) {
                        if (b > visualLeft) visualLeft = b;
                        if (b < visualRight) visualRight = b;
                        hasSignal = true;
                    }
                }

                // --- STEP B: DRAW MERGED BLOCK ---
                if (hasSignal) {
                    // Geometry Calculation
                    int x = startX + (7 - visualLeft) * cellWidth;
                    int y = startY + row * cellHeight;
                    int columnsWide = (visualLeft - visualRight + 1);
                    int totalWidth = columnsWide * cellWidth;

                    QRect mergedRect(x, y, totalWidth, cellHeight);

                    // 1. Draw Colored Background
                    QColor bg = getColorForSignal(signal);
                    int brightness = (bg.red() * 299 + bg.green() * 587 + bg.blue() * 114) / 1000;

                    painter.setPen(Qt::NoPen);
                    painter.setBrush(bg);
                    painter.drawRect(mergedRect);

                    // 2. Draw Internal Bit Separators
                    QColor sepColor = brightness > 128 ? QColor(0, 0, 0, 40) : QColor(255, 255, 255, 40);
                    painter.setPen(QPen(sepColor, 1));
                    painter.setBrush(Qt::NoBrush);
                    
                    for (int i = 1; i < columnsWide; ++i) {
                        int lineX = x + (i * cellWidth);
                        painter.drawLine(lineX, y, lineX, y + cellHeight);
                    }

                    // 3. Draw Border around the block
                    QColor borderColor = brightness > 128 ? QColor(0, 0, 0, 80) : QColor(255, 255, 255, 80);
                    painter.setPen(QPen(borderColor, 1));
                    painter.drawRect(mergedRect);

                    // ---------------------------------------------------------
                    // 3. TEXT RENDERING (Smart Size & Wrap)
                    // ---------------------------------------------------------
                    
                    // A. Contrast Color Calculation
                    painter.setPen(brightness > 128 ? Qt::black : Qt::white);

                    // B. Font Setup (Reduced Size)
                    QFont f = painter.font();
                    // Reduced to 30% of height for better fit
                    f.setPixelSize(qMax(6, int(cellHeight * 0.20))); 
                    f.setBold(true);
                    painter.setFont(f);

                    // C. Smart Formatting Logic
                    QString orderText = signal->isBigEndian() ? "MSB to LSB" : "LSB to MSB";
                    QString mappedName = signal->name() + " (" + orderText + ")";
                    QString rawName = signal->name();
                    QString arrowName = QString("<-- %1 -->").arg(mappedName);
                    
                    // Measure widths to decide format
                    QFontMetrics fm(f);
                    int arrowWidth = fm.horizontalAdvance(arrowName);
                    int mappedWidth = fm.horizontalAdvance(mappedName);
                    int boxWidth = mergedRect.width();

                    // Logic: Only use arrows if there is plenty of space (text < 90% of box)
                    // Otherwise, fall back to just the name to prevent clipping
                    QString finalText;
                    if (arrowWidth < (boxWidth * 0.9)) {
                        finalText = arrowName;
                    } else if (mappedWidth < (boxWidth * 0.9)) {
                        finalText = mappedName;
                    } else {
                        finalText = rawName;
                    }

                    // D. Draw with Word Wrap
                    // Qt::TextWordWrap allows text to break into new lines if it hits the edge
                    // Qt::AlignCenter keeps it centered vertically and horizontally
                    painter.drawText(mergedRect, Qt::AlignCenter | Qt::TextWordWrap, finalText);
                }
            }
        }
    }
}

QSize BitMatrixWidget::sizeHint() const
{
    int byteCount = 8;
    if (_msg) {
        byteCount = (_msg->getDlc() > 8 ? _msg->getDlc() : 8);
    }
    if (byteCount > 64) byteCount = 64;

    int cellSize = _cellSize;
    int cellHeight = cellSize;
    int cellWidth = int(cellHeight * 1.8); // Match paintEvent's wider cells
    int labelSize = 40;
    int margin = 10;

    int w = margin + labelSize + 8 * cellWidth + margin;
    int h = margin + labelSize + byteCount * cellHeight + margin;
    return QSize(w, h);
}

BitMatrixWidget::BitInfo BitMatrixWidget::getBitInfo(int byteIndex, int bitIndex)
{
    BitInfo info = { nullptr, false, false, Qt::white };
    if (!_msg) return info;

    // Use a unified index for bit mapping
    // Byte 0, Bit 0 is Absolute Bit 0
    // Byte 0, Bit 7 is Absolute Bit 7
    // Byte 1, Bit 0 is Absolute Bit 8
    int absoluteBit = byteIndex * 8 + bitIndex;

    foreach (CanDbSignal *sig, _msg->getSignals()) {
        int start = sig->startBit();
        int len = sig->length();
        bool intel = !sig->isBigEndian();

        if (intel) {
            if (absoluteBit >= start && absoluteBit < start + len) {
                info.signal = sig;
                info.isMsb = (absoluteBit == start + len - 1);
                info.isLsb = (absoluteBit == start);
                info.color = getColorForSignal(sig);
                return info;
            }
        } else {
            // Motorola logic
            int currentBit = start;
            for (int i = 0; i < len; ++i) {
                if (absoluteBit == currentBit) {
                    info.signal = sig;
                    info.isMsb = (i == 0);
                    info.isLsb = (i == len - 1);
                    info.color = getColorForSignal(sig);
                    return info;
                }
                
                if ((currentBit % 8) == 0) {
                    currentBit += 15;
                } else {
                    currentBit--;
                }
            }
        }
    }

    return info;
}

QColor BitMatrixWidget::getColorForSignal(CanDbSignal *signal)
{
    if (!_msg) return Qt::white;
    int index = _msg->getSignals().indexOf(signal);
    if (index >= 0) {
        return _signalColors[index % _signalColors.size()];
    }
    return Qt::white;
}

int BitMatrixWidget::getVisualLeftMostBit(CanDbSignal *sig, int byteIndex)
{
    // Iterate from Left (7) to Right (0)
    for (int b = 7; b >= 0; --b) {
        if (getBitInfo(byteIndex, b).signal == sig) {
            return b;
        }
    }
    return -1;
}
