/*
  cangaroo CAN message layout visualization widget.
*/

#pragma once

#include <QWidget>
#include <QPainter>
#include <QList>
#include <QColor>
#include "core/CanDbMessage.h"
#include "core/CanDbSignal.h"

class BitMatrixWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BitMatrixWidget(QWidget *parent = nullptr);
    void setMessage(CanDbMessage *msg);
    void setCellSize(int px);
    void setCompactMode(bool compact);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CanDbMessage *_msg;
    QList<QColor> _signalColors;
    int _cellSize;
    bool _compactMode;

    struct BitInfo {
        CanDbSignal *signal;
        bool isMsb;
        bool isLsb;
        QColor color;
    };

    BitInfo getBitInfo(int byteIndex, int bitIndex);
    int getVisualLeftMostBit(CanDbSignal *sig, int byteIndex);
    QColor getColorForSignal(CanDbSignal *signal);
};
