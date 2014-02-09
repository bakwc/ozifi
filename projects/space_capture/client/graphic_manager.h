#pragma once

#include <unordered_map>

#include <QObject>
#include <QImage>
#include <QColor>
#include <QtOpenGL/QGLWidget>

struct TPlanetKey {
    size_t Type;
    size_t Diameter;
    QColor Color;
    size_t operator()(const TPlanetKey& key) const;
    bool operator()(const TPlanetKey& a, const TPlanetKey& b) const;
};

struct TPlanetGraphics {
    QImage Image;
    GLuint TextureId;
};

struct TShipKey {
    float Scale;
    QColor Color;
    size_t operator()(const TShipKey& key) const;
    bool operator()(const TShipKey& a, const TShipKey& b) const;
};

typedef std::unordered_map<TPlanetKey, TPlanetGraphics, TPlanetKey, TPlanetKey> TPlanetCache;
typedef std::unordered_map<TShipKey, QImage, TShipKey, TShipKey> TShipCache;
typedef std::unordered_map<float, QImage> TBackgroundCache;

class TGraphicManager : public QObject
{
    Q_OBJECT
public:
    explicit TGraphicManager(QObject *parent = 0);
    const TPlanetGraphics& GetImage(size_t planetType, size_t diameter, QColor color);
    const QImage& GetShip(float scale, QColor color);
    const QImage& GetBackground(float scale);
    void ClearCache();
private:
    std::vector<QImage> PlanetImages;
    QImage Ship;
    QImage Background;
    TPlanetCache PlanetCache;
    TBackgroundCache BackgroundCache;
    TShipCache ShipCache;
};
