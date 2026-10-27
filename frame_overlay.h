#ifndef FRAME_OVERLAY_H
#define FRAME_OVERLAY_H

#include <QImage>

struct FELD_MAPP;   // definiert in hand.h

// Zeichnet das Feldraster und die Farben der Felder in ein Kamerabild und speichert es.
// Es wird nichts neu berechnet: Gezeichnet wird, was in FELD_MAPP steht
// (felds[i].feld_x, felds[i].feld_y und felds[i].collor).
//
// collor: 0 = Haut (gruen), 1 = vielleicht Haut (orange), 2 = keine Haut (nicht eingefaerbt)
namespace FrameOverlay
{
    // Gibt das Bild mit Raster und Feldfarben zurueck (Standard: Felder 10x10 Pixel,
    // Bild dreifach vergroessert, damit man die Linien sieht).
    QImage drawFields(const QImage& source, const FELD_MAPP& mapp, int fieldSize = 10, int scale = 3);

    // Zeichnet und speichert nach <PROJECT_DIR>/Frames/frame_<frameNumber>.bmp.
    // Legt den Ordner an, falls er fehlt. Gibt false zurueck, wenn das Speichern fehlschlaegt.
    bool saveFrameWithFields(const QImage& source, const FELD_MAPP& mapp, unsigned int frameNumber);
}

#endif // FRAME_OVERLAY_H