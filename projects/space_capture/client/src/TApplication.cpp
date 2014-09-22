#include "TApplication.h"

using namespace gameplay;

TApplication game;

TApplication::TApplication()
    : Scene(NULL), Wireframe(false)
{
}

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

void TApplication::initialize() {
        Scene = gameplay::Scene::create();
        Scene->setAmbientColor(1.f, 1.f, 1.f);

        Camera* cam = gameplay::Camera::createPerspective(28, 1.7, 0.25, 100);
        Node* camNode = Scene->addNode("cam");
        camNode->setCamera(cam);
        Scene->setActiveCamera(cam);
        camNode->setTranslation(0, 2.3, 6.5);


        std::vector<float> vertices = BuildSphere(1.0, 60, 60);
        std::vector<VertexFormat::Element> elements;
        elements.push_back(VertexFormat::Element(VertexFormat::POSITION, 3));
        elements.push_back(VertexFormat::Element(VertexFormat::NORMAL, 3));
        elements.push_back(VertexFormat::Element(VertexFormat::TEXCOORD0, 2));
        Mesh* mesh = Mesh::createMesh(VertexFormat(&elements[0], elements.size()), vertices.size() / 8, false);
        mesh->setPrimitiveType(Mesh::TRIANGLES);
        mesh->setVertexData(&vertices[0], 0, vertices.size() / 8);

        Node* sphere = Scene->addNode("sphere");
        sphere->setModel(gameplay::Model::create(mesh));
        sphere->getModel()->setMaterial("res/test.material#test");
        sphere->setTranslation(0.4, 1.7, -1.5);
}


void TApplication::finalize() {
    SAFE_RELEASE(Scene);
}

void TApplication::update(float elapsedTime) {
    Scene->findNode("sphere")->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 12000.0f * 180.0f));
}

void TApplication::render(float elapsedTime) {
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);
    Scene->visit(this, &TApplication::drawScene);
}

bool TApplication::drawScene(Node* node) {
    Model* model = node->getModel();
    if (model)
    {
        model->draw(Wireframe);
    }
    return true;
}

void TApplication::keyEvent(Keyboard::KeyEvent evt, int key) {
    if (evt == Keyboard::KEY_PRESS) {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void TApplication::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
    switch (evt) {
    case Touch::TOUCH_PRESS:
        Wireframe = !Wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
