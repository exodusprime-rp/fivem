﻿// WARNING: Please don't edit this file. It was generated by C++/WinRT v1.0.190111.3

#pragma once

WINRT_EXPORT namespace winrt::Microsoft::Graphics::Canvas::Effects {

struct Matrix5x4;

}

namespace winrt::impl {

template <> struct category<Microsoft::Graphics::Canvas::Effects::Matrix5x4>{ using type = struct_category<float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float>; };
template <> struct name<Microsoft::Graphics::Canvas::Effects::Matrix5x4>{ static constexpr auto & value{ L"Microsoft.Graphics.Canvas.Effects.Matrix5x4" }; };

struct struct_Microsoft_Graphics_Canvas_Effects_Matrix5x4
{
    float M11;
    float M12;
    float M13;
    float M14;
    float M21;
    float M22;
    float M23;
    float M24;
    float M31;
    float M32;
    float M33;
    float M34;
    float M41;
    float M42;
    float M43;
    float M44;
    float M51;
    float M52;
    float M53;
    float M54;
};
template <> struct abi<Microsoft::Graphics::Canvas::Effects::Matrix5x4>{ using type = struct_Microsoft_Graphics_Canvas_Effects_Matrix5x4; };


}
