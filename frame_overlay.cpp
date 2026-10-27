#include "frame_overlay.h"

#include "hand.h"

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QPen>
#include <QString>

QImage FrameOverlay::drawFields(const QImage& source, const FELD_MAPP& mapp, int fieldSize, int scale)
{
    if (source.isNull() || fieldSize <= 0 || scale <= 0)
        return source;

    // Nur volle Felder zaehlen, Restpixel am rechten und unteren Rand werden ignoriert
    // (genau wie bei der Erstellung der Felder in Hand::getFelds).
    const int cols = source.width() / fieldSize;
    const int rows = source.height() / fieldSize;
    const int cell = fieldSize * scale;

    // Vergroessern ohne Glaetten, damit die Pixel scharf bleiben
    QImage out = source.convertToFormat(QImage::Format_RGB32)
                     .scaled(source.width() * scale, source.height() * scale,
                             Qt::IgnoreAspectRatio, Qt::FastTransformation);

    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Felder halbtransparent einfaerben, so wie sie im Struct stehen
    for (const FELD& feld : mapp.felds)
    {
        if (feld.collor != 0 && feld.collor != 1)
            continue;   // 2 = keine Haut, alles andere wird ignoriert

        if (feld.feld_x < 0 || feld.feld_x >= cols || feld.feld_y < 0 || feld.feld_y >= rows)
            continue;   // Feld liegt ausserhalb des Bildes

        const QColor color = (feld.collor == 0) ? QColor(0, 200, 0, 140)
                                                : QColor(255, 140, 0, 140);
        painter.fillRect(feld.feld_x * cell, feld.feld_y * cell, cell, cell, color);
    }

    // Raster darueberlegen
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int c = 0; c <= cols; ++c)
        painter.drawLine(c * cell, 0, c * cell, rows * cell);
    for (int r = 0; r <= rows; ++r)
        painter.drawLine(0, r * cell, cols * cell, r * cell);

    painter.end();
    return out;
}

bool FrameOverlay::saveFrameWithFields(const QImage& source, const FELD_MAPP& mapp, unsigned int frameNumber)
{
    const QString frameDir = QString::fromUtf8(PROJECT_DIR) + "/Frames";

    // Beim ersten Aufruf nach dem Programmstart werden alle alten Frames geloescht
    // (Dateien, die mit "frame_" beginnen). Danach wird neu gespeichert.
    static std::once_flag clearFlag;
    std::call_once(clearFlag, [&frameDir]()
    {
        QDir dir(frameDir);
        if (!dir.exists())
            return;

        const QStringList oldFrames = dir.entryList(QStringList() << "frame_*", QDir::Files);
        for (const QString& name : oldFrames)
        {
            if (!dir.remove(name))
                qDebug() << "Alter Frame konnte nicht geloescht werden:" << dir.filePath(name);
        }
    });

    QDir().mkpath(frameDir);

    const QString framePath = frameDir + "/frame_" + QString::number(frameNumber) + ".bmp";

    const QImage out = drawFields(source, mapp);
    if (!out.save(framePath))
    {
        qDebug() << "Frame konnte nicht gespeichert werden:" << framePath;
        return false;
    }

    return true;
}