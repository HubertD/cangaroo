#include "WaveformDialog.h"
#include "ui_WaveformDialog.h"
#include <core/CanDbMessage.h>
#include <core/CanDbSignal.h>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <cmath>

// ---------------------------------------------------------------------------
// Waveform preview widget — redraws on every config change
// ---------------------------------------------------------------------------

class WaveformPreviewWidget : public QWidget
{
public:
    explicit WaveformPreviewWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(140);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setConfig(const WaveformConfig &cfg) { _cfg = cfg; update(); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int W = width(), H = height();
        const int pad = 44;   // left margin for axis labels
        const int padR = 6;
        const int padT = 18, padB = 24;
        const int plotW = W - pad - padR;
        const int plotH = H - padT - padB;

        p.fillRect(rect(), QColor(24, 24, 36));
        // Plot area background
        p.fillRect(pad, padT, plotW, plotH, QColor(30, 30, 46));
        p.setPen(QPen(QColor(50, 50, 70)));
        p.drawRect(pad, padT, plotW, plotH);

        if (_cfg.type == WaveformConfig::None) {
            p.setPen(QColor(120, 120, 140));
            p.setFont(QFont("monospace", 9));
            p.drawText(QRect(pad, padT, plotW, plotH), Qt::AlignCenter,
                       "No waveform");
            return;
        }

        // Determine Y axis range
        double yMin = 0, yMax = 1;
        switch (_cfg.type) {
        case WaveformConfig::Square:
            yMin = std::min(_cfg.sqLowVal,  _cfg.sqHighVal);
            yMax = std::max(_cfg.sqLowVal,  _cfg.sqHighVal);
            break;
        case WaveformConfig::Sine:
            yMin = _cfg.sinOffset - std::abs(_cfg.sinAmplitude);
            yMax = _cfg.sinOffset + std::abs(_cfg.sinAmplitude);
            break;
        case WaveformConfig::Ramp:
            yMin = std::min(_cfg.rampStartVal, _cfg.rampEndVal);
            yMax = std::max(_cfg.rampStartVal, _cfg.rampEndVal);
            break;
        default: break;
        }
        if (std::abs(yMax - yMin) < 1e-9) { yMin -= 0.5; yMax += 0.5; }
        double yRange = yMax - yMin;
        double yPad = yRange * 0.12;
        yMin -= yPad; yMax += yPad; yRange = yMax - yMin;

        // Helper lambdas
        auto toY = [&](double v) -> double {
            return padT + plotH * (1.0 - (v - yMin) / yRange);
        };
        auto toX = [&](double t_norm) -> double {   // t_norm in [0,2] periods
            return pad + t_norm / 2.0 * plotW;
        };

        // Grid — dotted horizontal lines at yMin, 0, yMax
        p.setPen(QPen(QColor(55, 55, 75), 1, Qt::DotLine));
        for (double gv : {yMin, (yMin+yMax)/2.0, yMax}) {
            double gy = toY(gv);
            p.drawLine(QLineF(pad, gy, pad + plotW, gy));
        }
        // Vertical grid at 0.5T and 1T and 1.5T
        for (double tx : {0.5, 1.0, 1.5}) {
            double gx = toX(tx);
            p.drawLine(QLineF(gx, padT, gx, padT + plotH));
        }

        // Y-axis labels
        p.setFont(QFont("monospace", 8));
        p.setPen(QColor(140, 140, 160));
        auto drawYLabel = [&](double v) {
            QString s = QString::number(v, 'g', 4);
            QRect r(0, (int)toY(v) - 8, pad - 3, 16);
            p.drawText(r, Qt::AlignRight | Qt::AlignVCenter, s);
        };
        drawYLabel(yMax - yPad);
        drawYLabel(yMin + yPad);
        if (yMin < 0 && yMax > 0) drawYLabel(0.0);

        // X-axis label: period marker
        p.setPen(QColor(100, 100, 130));
        p.setFont(QFont("monospace", 7));
        p.drawText(QRect((int)toX(0)-20, padT+plotH+3, 40, 14), Qt::AlignCenter, "0");
        p.drawText(QRect((int)toX(1)-20, padT+plotH+3, 40, 14), Qt::AlignCenter, "T");
        p.drawText(QRect((int)toX(2)-20, padT+plotH+3, 40, 14), Qt::AlignCenter, "2T");

        // --- Draw waveform (2 periods, normalized t in [0,2]) ---
        QPainterPath path;
        bool first = true;

        auto addPoint = [&](double t_norm, double v) {
            double x = toX(t_norm);
            double y = toY(v);
            if (first) { path.moveTo(x, y); first = false; }
            else        { path.lineTo(x, y); }
        };

        const int steps = plotW * 2;
        for (int i = 0; i <= steps; ++i) {
            double t_norm = 2.0 * i / steps;   // 0..2 periods
            double val = 0.0;
            switch (_cfg.type) {
            case WaveformConfig::Square: {
                double ton  = _cfg.sqTon_ms;
                double toff = _cfg.sqToff_ms;
                double period = ton + toff;
                if (period <= 0) break;
                double t_cyc = std::fmod(t_norm * period, period);
                val = (t_cyc < ton) ? _cfg.sqHighVal : _cfg.sqLowVal;
                break;
            }
            case WaveformConfig::Sine: {
                double phase = _cfg.sinPhase_deg * M_PI / 180.0;
                val = _cfg.sinOffset + _cfg.sinAmplitude *
                      std::sin(2.0 * M_PI * t_norm + phase);
                break;
            }
            case WaveformConfig::Ramp: {
                double rise = _cfg.rampRise_ms;
                double hold = _cfg.rampHold_ms;
                double cycle = rise + hold;
                if (cycle <= 0) break;
                double t_cyc = std::fmod(t_norm * cycle, cycle);
                if (t_cyc <= rise && rise > 0) {
                    val = _cfg.rampStartVal +
                          (_cfg.rampEndVal - _cfg.rampStartVal) * (t_cyc / rise);
                } else {
                    val = _cfg.rampEndVal;
                }
                break;
            }
            default: break;
            }
            addPoint(t_norm, val);
        }

        QPen wavePen;
        QString typeLabel;
        switch (_cfg.type) {
        case WaveformConfig::Square: wavePen = QPen(QColor(80, 160, 255), 2); typeLabel = "Square"; break;
        case WaveformConfig::Sine:   wavePen = QPen(QColor(80, 200, 120), 2); typeLabel = "Sine";   break;
        case WaveformConfig::Ramp:   wavePen = QPen(QColor(255, 180, 40),  2); typeLabel = "Ramp";   break;
        default: break;
        }
        p.setPen(wavePen);
        p.drawPath(path);

        // Bottom info line
        p.setPen(QColor(180, 180, 180));
        p.setFont(QFont("monospace", 8));
        QString info;
        switch (_cfg.type) {
        case WaveformConfig::Square:
            info = QString("Square  High=%1  Low=%2  TON=%3ms  TOFF=%4ms  T=%5ms")
                .arg(_cfg.sqHighVal,0,'g',4).arg(_cfg.sqLowVal,0,'g',4)
                .arg(_cfg.sqTon_ms,0,'f',1).arg(_cfg.sqToff_ms,0,'f',1)
                .arg(_cfg.sqTon_ms + _cfg.sqToff_ms,0,'f',1);
            break;
        case WaveformConfig::Sine:
            info = QString("Sine  A=%1  Offset=%2  T=%3ms  φ=%4°")
                .arg(_cfg.sinAmplitude,0,'g',4).arg(_cfg.sinOffset,0,'g',4)
                .arg(_cfg.sinPeriod_ms,0,'f',1).arg(_cfg.sinPhase_deg,0,'f',1);
            break;
        case WaveformConfig::Ramp:
            info = QString("Ramp  Start=%1  End=%2  Rise=%3ms  Hold=%4ms")
                .arg(_cfg.rampStartVal,0,'g',4).arg(_cfg.rampEndVal,0,'g',4)
                .arg(_cfg.rampRise_ms,0,'f',1).arg(_cfg.rampHold_ms,0,'f',1);
            break;
        default: break;
        }
        p.drawText(QRect(pad, H - padB + 4, W - pad, padB - 4), Qt::AlignLeft, info);
    }

private:
    WaveformConfig _cfg;
};

// ---------------------------------------------------------------------------
// WaveformDialog
// ---------------------------------------------------------------------------

WaveformDialog::WaveformDialog(CanDbMessage *dbMsg,
                               const QMap<QString, WaveformConfig> &existing,
                               QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WaveformDialog)
    , _dbMsg(dbMsg)
    , _waveforms(existing)
{
    ui->setupUi(this);
    setWindowTitle(tr("Configure Waveforms — %1 (0x%2)")
        .arg(dbMsg->getName())
        .arg(QString("%1").arg(dbMsg->getRaw_id(), 3, 16, QChar('0')).toUpper()));

    // Embed preview widget
    _preview = new WaveformPreviewWidget(this);
    QVBoxLayout *previewLayout = new QVBoxLayout(ui->previewContainer);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->addWidget(_preview);

    // Populate signal list
    CanDbSignalList sigList = dbMsg->getSignals();
    for (CanDbSignal *sig : sigList) {
        if (sig) ui->listSignals->addItem(sig->name());
    }
    if (ui->listSignals->count() > 0)
        ui->listSignals->setCurrentRow(0);

    // Type combo → switch stacked page
    connect(ui->comboType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveformDialog::onTypeChanged);

    // All parameter spin-boxes share the same slot
    auto anyChange = [this](double) { onAnyParamChanged(); };

    connect(ui->spinSqHigh,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSqLow,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSqTon,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSqToff,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);

    connect(ui->spinSinAmp,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSinOffset,QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSinPeriod,QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinSinPhase, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);

    connect(ui->spinRampStart,QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinRampEnd,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinRampRise, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);
    connect(ui->spinRampHold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, anyChange);

    connect(ui->listSignals, &QListWidget::currentTextChanged,
            this, &WaveformDialog::onSignalSelectionChanged);
    connect(ui->btnApplyToAll, &QPushButton::clicked, this, &WaveformDialog::onApplyToAll);

    // Load the first signal's existing config directly — do NOT call
    // onSignalSelectionChanged() here because that saves before loading and
    // would overwrite the caller-supplied existing configs with default values.
    loadSignalConfig(currentSignalName());
}

WaveformDialog::~WaveformDialog()
{
    delete ui;
}

QString WaveformDialog::currentSignalName() const
{
    QListWidgetItem *item = ui->listSignals->currentItem();
    return item ? item->text() : QString();
}

// ---------------------------------------------------------------------------

void WaveformDialog::loadSignalConfig(const QString &signalName)
{
    _loading = true;

    WaveformConfig cfg;
    if (_waveforms.contains(signalName))
        cfg = _waveforms[signalName];

    // Type + stacked page
    ui->comboType->setCurrentIndex((int)cfg.type);
    ui->stackedParams->setCurrentIndex((int)cfg.type);

    // Square
    ui->spinSqHigh->setValue(cfg.sqHighVal);
    ui->spinSqLow->setValue(cfg.sqLowVal);
    ui->spinSqTon->setValue(cfg.sqTon_ms);
    ui->spinSqToff->setValue(cfg.sqToff_ms);
    ui->lblSqPeriod->setText(tr("%1 ms  (TON + TOFF)")
        .arg(cfg.sqTon_ms + cfg.sqToff_ms, 0, 'f', 1));

    // Sine
    ui->spinSinAmp->setValue(cfg.sinAmplitude);
    ui->spinSinOffset->setValue(cfg.sinOffset);
    ui->spinSinPeriod->setValue(cfg.sinPeriod_ms);
    ui->spinSinPhase->setValue(cfg.sinPhase_deg);

    // Ramp
    ui->spinRampStart->setValue(cfg.rampStartVal);
    ui->spinRampEnd->setValue(cfg.rampEndVal);
    ui->spinRampRise->setValue(cfg.rampRise_ms);
    ui->spinRampHold->setValue(cfg.rampHold_ms);
    ui->lblRampCycle->setText(tr("%1 ms  (Rise + Hold)")
        .arg(cfg.rampRise_ms + cfg.rampHold_ms, 0, 'f', 1));

    // Unit label
    if (_dbMsg) {
        CanDbSignalList sl = _dbMsg->getSignals();
        for (CanDbSignal *sig : sl) {
            if (sig && sig->name() == signalName) {
                QString u = sig->getUnit();
                ui->lblUnit->setText(u.isEmpty() ? "" : QString("[ %1 ]").arg(u));
                break;
            }
        }
    }

    _loading = false;
    updatePreview();
}

void WaveformDialog::saveCurrentSignalConfig()
{
    QString sigName = currentSignalName();
    if (sigName.isEmpty()) return;

    WaveformConfig cfg;
    cfg.type = (WaveformConfig::Type)ui->comboType->currentIndex();

    cfg.sqHighVal  = ui->spinSqHigh->value();
    cfg.sqLowVal   = ui->spinSqLow->value();
    cfg.sqTon_ms   = ui->spinSqTon->value();
    cfg.sqToff_ms  = ui->spinSqToff->value();

    cfg.sinAmplitude = ui->spinSinAmp->value();
    cfg.sinOffset    = ui->spinSinOffset->value();
    cfg.sinPeriod_ms = ui->spinSinPeriod->value();
    cfg.sinPhase_deg = ui->spinSinPhase->value();

    cfg.rampStartVal = ui->spinRampStart->value();
    cfg.rampEndVal   = ui->spinRampEnd->value();
    cfg.rampRise_ms  = ui->spinRampRise->value();
    cfg.rampHold_ms  = ui->spinRampHold->value();

    _waveforms[sigName] = cfg;
}

// ---------------------------------------------------------------------------

void WaveformDialog::onSignalSelectionChanged()
{
    saveCurrentSignalConfig();
    loadSignalConfig(currentSignalName());
}

void WaveformDialog::onTypeChanged(int index)
{
    if (_loading) return;
    ui->stackedParams->setCurrentIndex(index);
    saveCurrentSignalConfig();
    updatePreview();
}

void WaveformDialog::onAnyParamChanged()
{
    if (_loading) return;

    // Keep computed period / cycle labels up-to-date
    ui->lblSqPeriod->setText(tr("%1 ms  (TON + TOFF)")
        .arg(ui->spinSqTon->value() + ui->spinSqToff->value(), 0, 'f', 1));
    ui->lblRampCycle->setText(tr("%1 ms  (Rise + Hold)")
        .arg(ui->spinRampRise->value() + ui->spinRampHold->value(), 0, 'f', 1));

    saveCurrentSignalConfig();
    updatePreview();
}

void WaveformDialog::onApplyToAll()
{
    saveCurrentSignalConfig();
    QString sigName = currentSignalName();
    if (sigName.isEmpty() || !_waveforms.contains(sigName)) return;
    WaveformConfig cfg = _waveforms[sigName];

    CanDbSignalList sl = _dbMsg->getSignals();
    for (CanDbSignal *s : sl) {
        if (s) _waveforms[s->name()] = cfg;
    }
}

void WaveformDialog::updatePreview()
{
    WaveformConfig cfg;
    cfg.type = (WaveformConfig::Type)ui->comboType->currentIndex();

    cfg.sqHighVal  = ui->spinSqHigh->value();
    cfg.sqLowVal   = ui->spinSqLow->value();
    cfg.sqTon_ms   = ui->spinSqTon->value();
    cfg.sqToff_ms  = ui->spinSqToff->value();

    cfg.sinAmplitude = ui->spinSinAmp->value();
    cfg.sinOffset    = ui->spinSinOffset->value();
    cfg.sinPeriod_ms = ui->spinSinPeriod->value();
    cfg.sinPhase_deg = ui->spinSinPhase->value();

    cfg.rampStartVal = ui->spinRampStart->value();
    cfg.rampEndVal   = ui->spinRampEnd->value();
    cfg.rampRise_ms  = ui->spinRampRise->value();
    cfg.rampHold_ms  = ui->spinRampHold->value();

    _preview->setConfig(cfg);
}
