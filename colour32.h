#pragma once

#include <math.h>

#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF
#define CLEAR 0x00000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF
#define YELLOW 0xFFFFFF00
#define MAGENTA 0xFFFF00FF
#define CYAN 0xFF00FFFF
#define EMPTY 0x00
#define FULL 0xFF

typedef unsigned char channel;
typedef unsigned long uint32;

/// <summary>
/// Converts given H, S, V arguments, as well as an optional Alpha, to a 32-bit color integer.
/// </summary>
/// <param name="H">: Hue value.</param>
/// <param name="S">: Saturation value. (0-100)</param>
/// <param name="V">: Vibrance value. (0-100)</param>
/// <param name="A">: Alpha value. (0-255)</param>
/// <returns>
/// Unsigned 32-bit integer that represents an ARGB color. If S or V are out of bounds, returns 0x00000000. (CLEAR)
/// </returns>
uint32 HSVtoARGB(double H, double S, double V, channel A = FULL)
{
    if (S > 100 || S < 0 || V > 100 || V < 0)
        return 0x00000000;

    H = 360 * (H < 0) + (1 - 2 * (H < 0)) * fabs(fmod(H, 360));

    double s = S / 100;
    double v = V / 100;
    double c = s * v;
    double x = c * (1 - fabs(fmod(H / 60.0, 2) - 1));
    double m = v - c;
    double r, g, b;

    if (H >= 0 && H < 60)
        r = c, g = x, b = 0;
    else if (H >= 60 && H < 120)
        r = x, g = c, b = 0;
    else if (H >= 120 && H < 180)
        r = 0, g = c, b = x;
    else if (H >= 180 && H < 240)
        r = 0, g = x, b = c;
    else if (H >= 240 && H < 300)
        r = x, g = 0, b = c;
    else
        r = c, g = 0, b = x;

    channel R = (r + m) * 255;
    channel G = (g + m) * 255;
    channel B = (b + m) * 255;

    return ((A << 24) | (R << 16) | (G << 8) | (B));
}

void ARGBtoHSV(double *outH, double *outV, double *outS)
{

}

/// <summary>
/// A simple class to hold a 32bit integer as a color.
/// </summary>
class colour32
{
protected:

    /// <summary>
    /// The integer value of the colour.
    /// </summary>
    const uint32 integer{ CLEAR };

    /// <summary>
    /// The Hue, Saturation, and Vibrance values used to set the colour. 
    /// If not used to set the colour, these are calculated from the integer value.
    /// </summary>
    double H{ DBL_MAX }, S{ DBL_MAX }, V{ DBL_MAX };

public:

    /// <summary>
    /// Constructs a 32-bit colour using an 32-bit integer.
    /// </summary>
    /// <param name="integer">The integer value used to construct the colour.</param>
    colour32(const uint32 value)
        : integer(value)
    {
    }

    /// <summary>
    /// Creates a 32-bit ARGB colour using R, G, B components, and an optional Alpha.
    /// </summary>
    /// <param name="red">: The 8-bit red component of the colour. (0-255)</param>
    /// <param name="green">: The 8-bit green component of the colour. (0-255)</param>
    /// <param name="blue">: The 8-bit blue component of the colour. (0-255)</param>
    /// <param name="alpha">: The 8-bit alpha component of the colour. (0-255) Defaults to 255.</param>
    colour32(channel red, channel green, channel blue, channel alpha=FULL)
        : integer((alpha << 24) | (red << 16) | (green << 8) | (blue))
    {
    }

    /// <summary>
    /// Creates a 32-bit ARGB colour using H, S, V components.
    /// </summary>
    /// <param name="hue">: The floating-point hue component of the colour. </param>
    /// <param name="saturation">: The floating-point saturation component of the colour. (0.0-100.0)</param>
    /// <param name="vibrance">: The floating-point vibrance component of the colour. (0.0-100.0)</param>
    colour32(double hue, double saturation, double vibrance)
        : integer(HSVtoARGB(hue, saturation, vibrance))
    {
        H = hue;
        S = saturation;
        V = vibrance;
    }

    /// <summary>
    /// Returns the 8-bit alpha component of the colour.  (0-255)
    /// </summary>
    const channel a()
    {
        uint32 aMask = 0xFF000000;
        return (integer & aMask) >> 24;
    }

    /// <summary>
    /// Returns the 8-bit red component of the colour. (0-255)
    /// </summary>
    const channel r()
    {
        uint32 rMask = 0xFF0000;
        return (integer & rMask) >> 16;
    }

    /// <summary>
    /// Returns the 8-bit greeb component of the colour.  (0-255)
    /// </summary>
    const channel g()
    {
        uint32 gMask = 0xFF00;
        return (integer & gMask) >> 8;
    }

    /// <summary>
    /// Returns the 8-bit blue component of the colour.  (0-255)
    /// </summary>
    const channel b()
    {
        uint32 bMask = 0xFF;
        return (integer & bMask);
    }

    /// <summary>
    /// Returns the integer value of the colour. (0-4294967295)
    /// </summary>
    const uint32 argb()
    {
        return integer;
    }

    /// <summary>
    /// Returns the integer value of the colour. (0-4294967295)
    /// </summary>
    const double h()
    {
        return H;
    }

    const double s()
    {
        return S;
    }

    const double v()
    {
        return V;
    }
};

