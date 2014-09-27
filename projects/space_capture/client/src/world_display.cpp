#include "world_display.h"
#include "TApplication.h"

using namespace gameplay;

void AddSpherePoint(std::vector<float>::iterator& v, int r, int s, float R, float S, float radius) {
    float const y = sin( -M_PI_2 + M_PI * r * R );
    float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
    float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
    *v++ = x * radius;
    *v++ = y * radius;
    *v++ = z * radius;

    *v++ = x;
    *v++ = y;
    *v++ = z;

    *v++ = s*S;
    *v++ = r*R;
}

std::vector<float> BuildSphere(float radius, unsigned int rings, unsigned int sectors) {
    std::vector<float> vertices;
    float const R = 1./(float)(rings-1);
    float const S = 1./(float)(sectors-1);
    vertices.resize(rings * sectors * 8 * 6);
    std::vector<float>::iterator v = vertices.begin();
    for(int r = 0; r < rings - 1; r++) {
        for(int s = 0; s < sectors; s++) {
            AddSpherePoint(v, r, s, R, S, radius);
            AddSpherePoint(v, r + 1, s, R, S, radius);
            AddSpherePoint(v, r, s + 1, R, S, radius);

            AddSpherePoint(v, r, s + 1, R, S, radius);
            AddSpherePoint(v, r + 1, s, R, S, radius);
            AddSpherePoint(v, r + 1, s + 1, R, S, radius);
        }
    }
    return vertices;
}

TWorldDisplay::TWorldDisplay(TWorld *world, gameplay::Scene* scene, TApplication* app)
    : World(world)
    , Scene(scene)
    , Application(app)
    , Ang(0)
{
    std::vector<float> vertices = BuildSphere(1.0, 20, 20);
    std::vector<VertexFormat::Element> elements;
    elements.push_back(VertexFormat::Element(VertexFormat::POSITION, 3));
    elements.push_back(VertexFormat::Element(VertexFormat::NORMAL, 3));
    elements.push_back(VertexFormat::Element(VertexFormat::TEXCOORD0, 2));
    Mesh* mesh = Mesh::createMesh(VertexFormat(&elements[0], elements.size()), vertices.size() / 8, false);
    mesh->setPrimitiveType(Mesh::TRIANGLES);
    mesh->setVertexData(&vertices[0], 0, vertices.size() / 8);

    Sphere = Scene->addNode("sphere");
    Sphere->setModel(gameplay::Model::create(mesh));
    Sphere->getModel()->setMaterial("res/test.material#base");

    Ship = SpriteBatch::create("res/ship.png");

    Font = Font::create("res/font-bold.gpb");
}

void TWorldDisplay::Draw(float elapsedTime) {
    Ang += elapsedTime * 0.0004;

    if (World->RoundStartsAt != uint8_t(-1)) {
        DrawRoundRestart(World->RoundStartsAt);
    } else if (World->WaitingPlayers) {
        DrawWaitingPlayers();
    } else {
        for (size_t i = 0; i < World->Planets.size(); ++i) {
            DrawPlanet(World->Planets[i]);
        }
        Ship->start();
        for (size_t i = 0; i < World->Ships.size(); ++i) {
            DrawShip( World->Ships[i]);
        }
        Ship->finish();
        DrawPower();
        DrawSelection();
    }
}

inline gameplay::Vector3 GetColor(NSpace::EColor color) {
    switch (color) {
        case NSpace::CR_Cyan: return gameplay::Vector3(0, 1, 1);
        case NSpace::CR_Blue: return gameplay::Vector3(0, 0, 1);
        case NSpace::CR_Green: return gameplay::Vector3(0, 1, 0);
        case NSpace::CR_Red: return gameplay::Vector3(1, 0, 0);
        case NSpace::CR_White: return gameplay::Vector3(1, 0, 1);
        case NSpace::CR_Yellow: return gameplay::Vector3(1, 1, 0.0);
    }
    return gameplay::Vector3(0.4, 0.4, 0.4);
}

template<class T>
std::string t_to_string(T i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

void TWorldDisplay::DrawPlanet(const NSpace::TPlanet& planet) {

    float radius = planet.Radius * World->Scale;
    float x = 0.01 * World->Scale * (planet.X - 0.5 * WORLD_WIDTH);
    float y = 0.01 * World->Scale * (planet.Y - 0.5 * WORLD_HEIGHT);

    NSpace::EColor color = (NSpace::EColor)255;
    if (planet.PlayerId != -1) {
        color = World->IdToPlayer[planet.PlayerId]->Color;
    }

    Vector3 planetColor = GetColor(color);

    Sphere->getModel()->getMaterial()->getTechniqueByIndex(0)->getPassByIndex(0)->
            getParameter("u_ambientColor")->setVector3(planetColor);

    Vector3 pos(x, y, -10.0);
    Sphere->setTranslation(pos);
    Sphere->setScale(0.01 * radius, 0.01 * radius, 0.01 * radius);

    Sphere->setRotation(Vector3(0, 1, 0), Ang);


//    Sphere->getModel()->getMaterial()->getTechniqueByIndex(0)->getPassByIndex(0)->
//            getParameter("u_diffuseTexture")->setSampler(std::string(
//                "res/planet" + std::to_string(planet.Type) + ".png").c_str(), true);

//    Sphere->getModel()->getMaterial()->getTechniqueByIndex(0)->getPassByIndex(0)->
//            getParameter("u_diffuseTexture")->setSampler(std::string(
//                "res/planet" + std::to_string(0) + ".png").c_str(), true);

    Sphere->getModel()->draw();


    if (planet.PlayerId == -1 || planet.PlayerId == World->SelfId) {
        std::string text = t_to_string(planet.Energy);
        float fontX, fontY;
        unsigned int textWidth, textHeight;
        Font->start();
        Application->project(pos, fontX, fontY);
        Font->measureText(text.c_str(), Font->getSize(), &textWidth, &textHeight);
        fontY -= 0.5 * textHeight;
        fontX -= 0.5 * textWidth;
        Vector4 fontColor(1, 1, 1, 1);
        if (color == NSpace::CR_Yellow) {
            fontColor = Vector4(0, 0, 0, 1);
        }
        Font->drawText(text.c_str(), fontX, fontY, fontColor, Font->getSize());
        Font->finish();
    }

    if (planet.PlayerId == World->SelfId)

    if (planet.PlayerId == World->SelfId &&
        World->SelectedPlanets.find(planet.ID) != World->SelectedPlanets.end())
    {
//        Application->Scene->
//        pen.setColor(planetColor);
//        painter.setPen(pen);
//        painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
    }

//    if (planet.PlayerId == World->SelfId &&
//        World->SelectedPlanets.find(planet.ID) != World->SelectedPlanets.end())
//    {
//        pen.setColor(planetColor);
//        painter.setPen(pen);
//        painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
//    } else if (World->SelectedTarget.is_initialized() &&
//               planet.ID == *World->SelectedTarget)
//    {
//        NSpace::TPlayer* selfPlayer = World->SelfPlayer();
//        if (selfPlayer != nullptr) {
//            planetColor = GetQColor(selfPlayer->Color);
//            pen.setColor(planetColor);
//            painter.setPen(pen);
//            painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
//        }
//    }
}

void TWorldDisplay::DrawShip(const NSpace::TShip& ship) {

    float x = 0.01 * World->Scale * (ship.X - 0.5 * WORLD_WIDTH);
    float y = 0.01 * World->Scale * (ship.Y - 0.5 * WORLD_HEIGHT);

    Vector3 planetColor = GetColor(World->IdToPlayer[ship.PlayerID]->Color);
    Vector3 pos(x, y, -10.0);

    float x2d, y2d;
    Application->project(pos, x2d, y2d);

    Ship->draw(Vector3(x2d, y2d, 0), Rectangle(0, 0, 8, 13), Vector2(8, 13),
               Vector4(planetColor.x, planetColor.y, planetColor.z, 1),
               Vector2(0.5f, 0.5f), - float(ship.Angle) / 100.0 - M_PI_2);
}

void TWorldDisplay::DrawSelection() {
//    if (World->Selection.is_initialized()) {
//        painter.setPen(Qt::cyan);
//        int x = std::min(World->Selection->From.x(), World->Selection->To.x());
//        int y = std::min(World->Selection->From.y(), World->Selection->To.y());
//        int w = abs(World->Selection->From.x() - World->Selection->To.x());
//        int h = abs(World->Selection->From.y() - World->Selection->To.y());
//        painter.drawRect(x, y, w, h);
//    }
}

void TWorldDisplay::DrawPower() {
//    int x = 0.97 * WORLD_WIDTH * World->Scale + World->OffsetX;
//    int y = 0.78 * WORLD_HEIGHT * World->Scale + World->OffsetY;
//    int width = 0.012 * WORLD_WIDTH * World->Scale;
//    int height = 0.2 * WORLD_HEIGHT * World->Scale;
//    QPen pen(Qt::white);
//    pen.setWidth(2);
//    painter.setPen(pen);
//    painter.drawRect(x, y, width, height);

//    int filled = 0.01 * World->Power * height;
//    painter.fillRect(x, y + height - filled, width, filled, Qt::white);
}

void TWorldDisplay::DrawRoundRestart(int restartTime) {
//    int x = 0.5 * WORLD_WIDTH * World->Scale + World->OffsetX;
//    int y = 0.5 * WORLD_HEIGHT * World->Scale + World->OffsetY;
//    QPen pen(Qt::white);
//    painter.setPen(pen);
//    painter.setFont(QFont("arial", 28));
//    painter.drawText(x, y, QString("%1").arg(restartTime));
}

void TWorldDisplay::DrawWaitingPlayers() {
//    int x = 0.5 * WORLD_WIDTH * World->Scale + World->OffsetX;
//    int y = 0.5 * WORLD_HEIGHT * World->Scale + World->OffsetY;
//    QPen pen(Qt::white);
//    painter.setPen(pen);
//    painter.setFont(QFont("arial", 28));
//    QString text = "Waiting for players...";
//    QFontMetrics fm(painter.font());
//    int textWidth = fm.width(text);
//    painter.drawText(x - textWidth / 2, y, text);
}
