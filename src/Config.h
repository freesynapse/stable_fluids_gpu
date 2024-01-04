#pragma once

#include <stdint.h>
#include <typeinfo>
#include <iostream>
#include <algorithm>

#include <synapse/External/imgui/imgui.h>

//
template<typename T>
class Property
{
public:
    Property() {}
    Property(const T &_val, const T &_min, const T &_max) :
        m_val(_val), m_min(_min), m_max(_max)
    {}

    //
    T operator()() const { return m_val; }
    void operator=(const T &_val) { m_val = std::clamp(_val, m_min, m_max); }

    //
    void __debug_print()
    {
        printf("Property of type <%s>:\n", typeid(T).name());
        std::cout << "    val: " << m_val << std::endl;
        std::cout << "    min: " << m_min << std::endl;
        std::cout << "    max: " << m_max << std::endl;
    }

public:
    T m_val;
    T m_max;
    T m_min;

};

//
class Config
{
public:
    static void settingsDialog();

public:
    // solver parameters
    static Property<float> velocityDissipation;
    static Property<float> densityDissipation;
    static Property<float> vorticityDissipation;
    static Property<float> vorticityConfinement;
    static Property<int> jacobiIterCount;

    // interaction
    static Property<float> forceRadius;
    static Property<float> forceMultiplier;
    static Property<float> densityRadius;

    static Property<bool> isRunning;
    static Property<bool> showQuivers;

    // renderer
    static Property<bool> renderRGB;
    static Property<int> quiverSamplingRate;

};
