#pragma once

#include <cinttypes>
#include <stdlib.h>

#include "../../utils/saveload.h"

const float WORLD_WIDTH = 1000;
const float WORLD_HEIGHT = 800;
const size_t MAX_PLANET_TYPES = 5;
const size_t MAX_PLAYERS = 6;
const char* const CONTROL_SERVER_ADDRESS = "127.0.0.1";
const size_t CONTROL_SERVER_PORT = 8100;

const float PLAYER_PLANET_RADIUS = 30.0f;
const float MINIMUM_PLAYER_PLANET_DISTANCE = 100.0f;
const float MINIMUM_PLANET_DISTANCE = 31.0f;
const float PLAYER_PLANET_ENERGY = 100.0f;
const size_t PLANETS_MAX_NUMBER = 20;
const size_t PLANETS_MIN_NUMBER = 10;
const float PLANET_MAX_RADIUS = 50.0f;
const float PLANET_MIN_RADIUS = 10.0f;
const float PLANET_MAX_ENERGY = 50.0f;
const float PLANET_MIN_ENERGY = 0.0f;
const size_t ROUND_RESTART_TIME = 7;

struct TPointF {
    float X;
    float Y;
    TPointF()
        : X(0)
        , Y(0)
    {
    }
    TPointF(const TPointF& other)
        : X(other.X)
        , Y(other.Y)
    {
    }
    TPointF(float x, float y)
        : X(x)
        , Y(y)
    {
    }
    TPointF& operator +=(const TPointF& other) {
        X += other.X;
        Y += other.Y;
        return *this;
    }
    TPointF& operator -=(const TPointF& other) {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }
    TPointF& operator /=(float number) {
        X /= number;
        Y /= number;
        return *this;
    }
    TPointF& operator *=(float number) {
        X *= number;
        Y *= number;
        return *this;
    }

    TPointF operator +(const TPointF& other) {
        return TPointF(X + other.X, Y + other.Y);
    }
    TPointF operator -(const TPointF& other) {
        return TPointF(X - other.X, Y - other.Y);
    }
    TPointF operator /(float number) {
        return TPointF(X / number, Y / number);
    }
    TPointF operator *(float number) {
        return TPointF(X * number, Y * number);
    }
    bool operator ==(const TPointF& other) {
        return X == other.X && Y == other.Y;
    }
    bool operator !=(const TPointF& other) {
        return X != other.X || Y != other.Y;
    }

    SAVELOAD(X, Y)
};

struct TPoint {
    TPoint()
        : X(0)
        , Y(0)
    {
    }
    TPoint(int x, int y)
        : X(x)
        , Y(y)
    {
    }

    int X;
    int Y;
    SAVELOAD(X, Y)
};

struct TSize {
    TSize()
        : Width(0)
        , Height(0)
    {
    }
    TSize(int width, int height)
        : Width(width)
        , Height(height)
    {
    }
    int Width;
    int Height;

    SAVELOAD(Width, Height)
};
