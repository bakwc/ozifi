#include <projects/space_capture/lib/defines.h>

#include "graphic_manager.h"

size_t TPlanetKeyHash::operator()(const TPlanetKey& key) const {
    return std::hash<size_t>()(key.Type) + std::hash<size_t>()(key.Diameter) +
            std::hash<int>()(key.Color.red()) +
            std::hash<int>()(key.Color.green()) +
            std::hash<int>()(key.Color.blue());
}

bool TPlanetKeyCompare::operator()(const TPlanetKey& a, const TPlanetKey& b) const {
    return a.Type == b.Type && a.Color == b.Color && a.Diameter == b.Diameter;
}

TGraphicManager::TGraphicManager(QObject *parent) :
    QObject(parent)
{
    for (size_t i = 0; i < MAX_PLANET_TYPES; ++i) {
        QString resourceName = QString(":/graphics/planet%1.png").arg(i);
        PlanetImages.push_back(QImage(resourceName));
    }
    ClearCache();
}

const QImage& TGraphicManager::GetImage(size_t planetType, size_t diameter, QColor color) {
    TPlanetKey key = {planetType, diameter, color};
    auto it = PlanetCache.find(key);
    if (it == PlanetCache.end()) {
        QImage image = PlanetImages[planetType].scaled(diameter, diameter, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        float cr, cg, cb;
        cr = (float)color.red() / 255.0;
        cg = (float)color.green() / 255.0;
        cb = (float)color.blue() / 255.0;
        for (size_t i = 0; i < image.width(); ++i) {
            for (size_t j = 0; j < image.height(); ++j) {
                QRgb p = image.pixel(i, j);
                QRgb newP = qRgb(cr * qRed(p), cg * qGreen(p), cb * qBlue(p));
                image.setPixel(i, j, newP);
            }
        }

        it = PlanetCache.insert(it, std::pair<TPlanetKey, QImage>(key, image));
    }
    return it->second;
}

void TGraphicManager::ClearCache() {
    PlanetCache.clear();
}
