/** This file was modified by me (Chris Laurel) from Johannes Gadjosik's C implementation
  * of the GUST86 theory of the Uranian satellites. The original copyright is retained
  * below. My modifications are:
  *
  * - Added interface for VESTA's Trajectory class
  * - Changed code to compute coordinates in J2000 ecliptic frame transformation
  * - Removed orbital element interpolation code
  * - Compute the velocity as well as the position
  */


/************************************************************************

COMPUTATION OF THE COORDINATES OF THE URANIAN SATELLITES (GUST86),
version 0.1 (1988,1995) by LASKAR J. and JACOBSON, R. can be found at
ftp://ftp.imcce.fr/pub/ephem/satel/gust86

I (Johannes Gajdosik) have just taken the Fortran code and data
obtained from above and rearranged it into this piece of software.

I can neigther allow nor forbid the usage of the GUST86 theory.
The copyright notice below covers not the works of LASKAR J. and JACOBSON, R.,
but just my work, that is the compilation of the GUST86 theory
into the software supplied in this file.


Copyright (c) 2005 Johannes Gajdosik

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

My implementation of GUST86 has the following modifications:
1) Rotate results to "dynamical equinox and ecliptic J2000",
   the reference frame of VSOP87 and VSOP87A:
   The rotation matrix Gust86ToJ2000 can be derived from gust86.f,
   the rotation J2000ToVsop87 can be derived from vsop87.doc.
2) units used in calculations: julian day, AU, rad
3) use the same function EllipticToRectangular that I use in TASS17.
4) calculate the orbital elements not for every new jd but rather reuse
   the previousely calculated elements if possible

****************************************************************/

#include "Gust86.h"
#include "Constants.h"
#include <vesta/Units.h>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static
const double fqn[5] = {4.44519055,
                       2.492952519,
                       1.516148111,
                       0.721718509,
                       0.46669212};
static
const double fqe[5] = {20.082*M_PI/(180*365.25),
                        6.217*M_PI/(180*365.25),
                        2.865*M_PI/(180*365.25),
                        2.078*M_PI/(180*365.25),
                        0.386*M_PI/(180*365.25)};
static
const double fqi[5] = {-20.309*M_PI/(180*365.25),
                        -6.288*M_PI/(180*365.25),
                        -2.836*M_PI/(180*365.25),
                        -1.843*M_PI/(180*365.25),
                        -0.259*M_PI/(180*365.25)};
static
const double phn[5] = {-0.238051,
                        3.098046,
                        2.285402,
                        0.856359,
                       -0.915592};
static
const double phe[5] = {0.611392,
                       2.408974,
                       2.067774,
                       0.735131,
                       0.426767};
static
const double phi[5] = {5.702313,
                       0.395757,
                       0.589326,
                       1.746237,
                       4.206896};


void
CalcGust86Elem(double t, Gust86Orbit::Satellite body, double elements[6])
{
    double an[5],ae[5],ai[5];

    for (int i = 0; i < 5; i++)
    {
        an[i] = fmod(fqn[i] * t + phn[i], 2*M_PI);
        ae[i] = fmod(fqe[i] * t + phe[i], 2*M_PI);
        ai[i] = fmod(fqi[i] * t + phi[i], 2*M_PI);
    }

    switch (body)
    {
    case Gust86Orbit::Miranda:
        elements[0] = 4.44352267
                  - cos(an[0]      - an[1] * 3. + an[2] * 2.) * 3.492e-5
                  + cos(an[0] * 2. - an[1] * 6. + an[2] * 4.) * 8.47e-6
                  + cos(an[0] * 3. - an[1] * 9. + an[2] * 6.) * 1.31e-6
                  - cos(an[0]      - an[1]                     ) * 5.228e-5
                  - cos(an[0] * 2. - an[1] * 2.                ) * 1.3665e-4;
        elements[1] =
                    sin(an[0]      - an[1] * 3. + an[2] * 2.) * .02547217
                  - sin(an[0] * 2. - an[1] * 6. + an[2] * 4.) * .00308831
                  - sin(an[0] * 3. - an[1] * 9. + an[2] * 6.) * 3.181e-4
                  - sin(an[0] * 4. - an[1] * 12 + an[2] * 8.) * 3.749e-5
                  - sin(an[0]      - an[1]                     ) * 5.785e-5
                  - sin(an[0] * 2. - an[1] * 2.                ) * 6.232e-5
                  - sin(an[0] * 3. - an[1] * 3.                ) * 2.795e-5
                  + t * 4.44519055 - .23805158;
        elements[2] = cos(ae[0]) * .00131238
                  + cos(ae[1]) * 7.181e-5
                  + cos(ae[2]) * 6.977e-5
                  + cos(ae[3]) * 6.75e-6
                  + cos(ae[4]) * 6.27e-6
                  + cos(an[0]) * 1.941e-4
                  - cos(-an[0]      + an[1] * 2.) * 1.2331e-4
                  + cos(an[0] * -2. + an[1] * 3.) *  3.952e-5;
        elements[3] = sin(ae[0]) * .00131238
                  + sin(ae[1]) * 7.181e-5
                  + sin(ae[2]) * 6.977e-5
                  + sin(ae[3]) * 6.75e-6
                  + sin(ae[4]) * 6.27e-6
                  + sin(an[0]) * 1.941e-4
                  - sin(-an[0]      + an[1] * 2.) * 1.2331e-4
                  + sin(an[0] * -2. + an[1] * 3.) * 3.952e-5;
        elements[4] = cos(ai[0]) * .03787171
                  + cos(ai[1]) * 2.701e-5
                  + cos(ai[2]) * 3.076e-5
                  + cos(ai[3]) * 1.218e-5
                  + cos(ai[4]) * 5.37e-6;
        elements[5] = sin(ai[0]) * .03787171
                  + sin(ai[1]) * 2.701e-5
                  + sin(ai[2]) * 3.076e-5
                  + sin(ai[3]) * 1.218e-5
                  + sin(ai[4]) * 5.37e-6;
        break;

    case Gust86Orbit::Ariel:
        elements[0] = 2.49254257
                  + cos(an[0] - an[1] * 3. + an[2] * 2.) * 2.55e-6
                  - cos(           an[1]      - an[2]     ) * 4.216e-5
                  - cos(           an[1] * 2. - an[2] * 2.) * 1.0256e-4;
        elements[1] =
                  - sin(an[0]      - an[1] * 3. + an[2] * 2.) * .0018605
                  + sin(an[0] * 2. - an[1] * 6. + an[2] * 4.) * 2.1999e-4
                  + sin(an[0] * 3. - an[1] * 9. + an[2] * 6.) * 2.31e-5
                  + sin(an[0] * 4. - an[1] * 12 + an[2] * 8.) * 4.3e-6
                  - sin(                an[1]      - an[2]     ) * 9.011e-5
                  - sin(                an[1] * 2. - an[2] * 2.) * 9.107e-5
                  - sin(                an[1] * 3. - an[2] * 3.) * 4.275e-5
                  - sin(                an[1] * 2.     - an[3] * 2.) * 1.649e-5
                  + t * 2.49295252 + 3.09804641;
        elements[2] = cos(ae[0]) * -3.35e-6
                  + cos(ae[1]) * .00118763
                  + cos(ae[2]) * 8.6159e-4
                  + cos(ae[3]) * 7.15e-5
                  + cos(ae[4]) * 5.559e-5
                  - cos(-an[1] + an[2] * 2.) * 8.46e-5
                  + cos(an[1] * -2. + an[2] * 3.) * 9.181e-5
                  + cos(-an[1] + an[3] * 2.) * 2.003e-5
                  + cos(an[1]) * 8.977e-5;
        elements[3] = sin(ae[0]) * -3.35e-6
                  + sin(ae[1]) * .00118763
                  + sin(ae[2]) * 8.6159e-4
                  + sin(ae[3]) * 7.15e-5
                  + sin(ae[4]) * 5.559e-5
                  - sin(-an[1] + an[2] * 2.) * 8.46e-5
                  + sin(an[1] * -2. + an[2] * 3.) * 9.181e-5
                  + sin(-an[1] + an[3] * 2.) * 2.003e-5
                  + sin(an[1]) * 8.977e-5;
        elements[4] = cos(ai[0]) * -1.2175e-4
                  + cos(ai[1]) * 3.5825e-4
                  + cos(ai[2]) * 2.9008e-4
                  + cos(ai[3]) * 9.778e-5
                  + cos(ai[4]) * 3.397e-5;
        elements[5] = sin(ai[0]) * -1.2175e-4
                  + sin(ai[1]) * 3.5825e-4
                  + sin(ai[2]) * 2.9008e-4
                  + sin(ai[3]) * 9.778e-5
                  + sin(ai[4]) * 3.397e-5;
        break;

    case Gust86Orbit::Umbriel:
        elements[0] = 1.5159549
                  + cos(an[2] - an[3] * 2. + ae[2]) * 9.74e-6
                  - cos(an[1] - an[2]) * 1.06e-4
                  + cos(an[1] * 2. - an[2] * 2.) * 5.416e-5
                  - cos(an[2] - an[3]) * 2.359e-5
                  - cos(an[2] * 2. - an[3] * 2.) * 7.07e-5
                  - cos(an[2] * 3. - an[3] * 3.) * 3.628e-5;
        elements[1] =
                    sin(an[0] - an[1] * 3. + an[2] * 2.) * 6.6057e-4
                  - sin(an[0] * 2. - an[1] * 6. + an[2] * 4.) * 7.651e-5
                  - sin(an[0] * 3. - an[1] * 9. + an[2] * 6.) * 8.96e-6
                  - sin(an[0] * 4. - an[1] * 12. + an[2] * 8.) * 2.53e-6
                  - sin(an[2] - an[3] * 4. + an[4] * 3.) * 5.291e-5
                  - sin(an[2] - an[3] * 2. + ae[4]) * 7.34e-6
                  - sin(an[2] - an[3] * 2. + ae[3]) * 1.83e-6
                  + sin(an[2] - an[3] * 2. + ae[2]) * 1.4791e-4
                  + sin(an[2] - an[3] * 2. + ae[1]) * -7.77e-6
                  + sin(an[1] - an[2]) * 9.776e-5
                  + sin(an[1] * 2. - an[2] * 2.) * 7.313e-5
                  + sin(an[1] * 3. - an[2] * 3.) * 3.471e-5
                  + sin(an[1] * 4. - an[2] * 4.) * 1.889e-5
                  - sin(an[2] - an[3]) * 6.789e-5
                  - sin(an[2] * 2. - an[3] * 2.) * 8.286e-5
                  + sin(an[2] * 3. - an[3] * 3.) * -3.381e-5
                  - sin(an[2] * 4. - an[3] * 4.) * 1.579e-5
                  - sin(an[2] - an[4]) * 1.021e-5
                  - sin(an[2] * 2. - an[4] * 2.) * 1.708e-5
                  + t * 1.51614811 + 2.28540169;
        elements[2] = cos(ae[0]) * -2.1e-7
                  - cos(ae[1]) * 2.2795e-4
                  + cos(ae[2]) * .00390469
                  + cos(ae[3]) * 3.0917e-4
                  + cos(ae[4]) * 2.2192e-4
                  + cos(an[1]) * 2.934e-5
                  + cos(an[2]) * 2.62e-5
                  + cos(-an[1] + an[2] * 2.) * 5.119e-5
                  - cos(an[1] * -2. + an[2] * 3.) * 1.0386e-4
                  - cos(an[1] * -3. + an[2] * 4.) * 2.716e-5
                  + cos(an[3]) * -1.622e-5
                  + cos(-an[2] + an[3] * 2.) * 5.4923e-4
                  + cos(an[2] * -2. + an[3] * 3.) * 3.47e-5
                  + cos(an[2] * -3. + an[3] * 4.) * 1.281e-5
                  + cos(-an[2] + an[4] * 2.) * 2.181e-5
                  + cos(an[2]) * 4.625e-5;
        elements[3] = sin(ae[0]) * -2.1e-7
                  - sin(ae[1]) * 2.2795e-4
                  + sin(ae[2]) * .00390469
                  + sin(ae[3]) * 3.0917e-4
                  + sin(ae[4]) * 2.2192e-4
                  + sin(an[1]) * 2.934e-5
                  + sin(an[2]) * 2.62e-5
                  + sin(-an[1] + an[2] * 2.) * 5.119e-5
                  - sin(an[1] * -2. + an[2] * 3.) * 1.0386e-4
                  - sin(an[1] * -3. + an[2] * 4.) * 2.716e-5
                  + sin(an[3]) * -1.622e-5
                  + sin(-an[2] + an[3] * 2.) * 5.4923e-4
                  + sin(an[2] * -2. + an[3] * 3.) * 3.47e-5
                  + sin(an[2] * -3. + an[3] * 4.) * 1.281e-5
                  + sin(-an[2] + an[4] * 2.) * 2.181e-5
                  + sin(an[2]) * 4.625e-5;
        elements[4] = cos(ai[0]) * -1.086e-5
                  - cos(ai[1]) * 8.151e-5
                  + cos(ai[2]) * .00111336
                  + cos(ai[3]) * 3.5014e-4
                  + cos(ai[4]) * 1.065e-4;
        elements[5] = sin(ai[0]) * -1.086e-5
                  - sin(ai[1]) * 8.151e-5
                  + sin(ai[2]) * .00111336
                  + sin(ai[3]) * 3.5014e-4
                  + sin(ai[4]) * 1.065e-4;
        break;

    case Gust86Orbit::Titania:
        elements[0] = .72166316
                  - cos(an[2] - an[3] * 2. + ae[2]) * 2.64e-6
                  - cos(an[3] * 2. - an[4] * 3. + ae[4]) * 2.16e-6
                  + cos(an[3] * 2. - an[4] * 3. + ae[3]) * 6.45e-6
                  - cos(an[3] * 2. - an[4] * 3. + ae[2]) * 1.11e-6
                  + cos(an[1] - an[3]) * -6.223e-5
                  - cos(an[2] - an[3]) * 5.613e-5
                  - cos(an[3] - an[4]) * 3.994e-5
                  - cos(an[3] * 2. - an[4] * 2.) * 9.185e-5
                  - cos(an[3] * 3. - an[4] * 3.) * 5.831e-5
                  - cos(an[3] * 4. - an[4] * 4.) * 3.86e-5
                  - cos(an[3] * 5. - an[4] * 5.) * 2.618e-5
                  - cos(an[3] * 6. - an[4] * 6.) * 1.806e-5;
        elements[1] =
                    sin(an[2] - an[3] * 4. + an[4] * 3.) * 2.061e-5
                  - sin(an[2] - an[3] * 2. + ae[4]) * 2.07e-6
                  - sin(an[2] - an[3] * 2. + ae[3]) * 2.88e-6
                  - sin(an[2] - an[3] * 2. + ae[2]) * 4.079e-5
                  + sin(an[2] - an[3] * 2. + ae[1]) * 2.11e-6
                  - sin(an[3] * 2. - an[4] * 3. + ae[4]) * 5.183e-5
                  + sin(an[3] * 2. - an[4] * 3. + ae[3]) * 1.5987e-4
                  + sin(an[3] * 2. - an[4] * 3. + ae[2]) * -3.505e-5
                  - sin(an[3] * 3. - an[4] * 4. + ae[4]) * 1.56e-6
                  + sin(an[1] - an[3]) * 4.054e-5
                  + sin(an[2] - an[3]) * 4.617e-5
                  - sin(an[3] - an[4]) * 3.1776e-4
                  - sin(an[3] * 2. - an[4] * 2.) * 3.0559e-4
                  - sin(an[3] * 3. - an[4] * 3.) * 1.4836e-4
                  - sin(an[3] * 4. - an[4] * 4.) * 8.292e-5
                  + sin(an[3] * 5. - an[4] * 5.) * -4.998e-5
                  - sin(an[3] * 6. - an[4] * 6.) * 3.156e-5
                  - sin(an[3] * 7. - an[4] * 7.) * 2.056e-5
                  - sin(an[3] * 8. - an[4] * 8.) * 1.369e-5
                  + t * .72171851 + .85635879;
        elements[2] = cos(ae[0]) * -2e-8
                  - cos(ae[1]) * 1.29e-6
                  - cos(ae[2]) * 3.2451e-4
                  + cos(ae[3]) * 9.3281e-4
                  + cos(ae[4]) * .00112089
                  + cos(an[1]) * 3.386e-5
                  + cos(an[3]) * 1.746e-5
                  + cos(-an[1] + an[3] * 2.) * 1.658e-5
                  + cos(an[2]) * 2.889e-5
                  - cos(-an[2] + an[3] * 2.) * 3.586e-5
                  + cos(an[3]) * -1.786e-5
                  - cos(an[4]) * 3.21e-5
                  - cos(-an[3] + an[4] * 2.) * 1.7783e-4
                  + cos(an[3] * -2. + an[4] * 3.) * 7.9343e-4
                  + cos(an[3] * -3. + an[4] * 4.) * 9.948e-5
                  + cos(an[3] * -4. + an[4] * 5.) * 4.483e-5
                  + cos(an[3] * -5. + an[4] * 6.) * 2.513e-5
                  + cos(an[3] * -6. + an[4] * 7.) * 1.543e-5;
        elements[3] = sin(ae[0]) * -2e-8
                  - sin(ae[1]) * 1.29e-6
                  - sin(ae[2]) * 3.2451e-4
                  + sin(ae[3]) * 9.3281e-4
                  + sin(ae[4]) * .00112089
                  + sin(an[1]) * 3.386e-5
                  + sin(an[3]) * 1.746e-5
                  + sin(-an[1] + an[3] * 2.) * 1.658e-5
                  + sin(an[2]) * 2.889e-5
                  - sin(-an[2] + an[3] * 2.) * 3.586e-5
                  + sin(an[3]) * -1.786e-5
                  - sin(an[4]) * 3.21e-5
                  - sin(-an[3] + an[4] * 2.) * 1.7783e-4
                  + sin(an[3] * -2. + an[4] * 3.) * 7.9343e-4
                  + sin(an[3] * -3. + an[4] * 4.) * 9.948e-5
                  + sin(an[3] * -4. + an[4] * 5.) * 4.483e-5
                  + sin(an[3] * -5. + an[4] * 6.) * 2.513e-5
                  + sin(an[3] * -6. + an[4] * 7.) * 1.543e-5;
        elements[4] = cos(ai[0]) * -1.43e-6
                  - cos(ai[1]) * 1.06e-6
                  - cos(ai[2]) * 1.4013e-4
                  + cos(ai[3]) * 6.8572e-4
                  + cos(ai[4]) * 3.7832e-4;
        elements[5] = sin(ai[0]) * -1.43e-6
                  - sin(ai[1]) * 1.06e-6
                  - sin(ai[2]) * 1.4013e-4
                  + sin(ai[3]) * 6.8572e-4
                  + sin(ai[4]) * 3.7832e-4;
        break;

    case Gust86Orbit::Oberon:
        elements[0] = .46658054
                  + cos(an[3] * 2. - an[4] * 3. + ae[4]) * 2.08e-6
                  - cos(an[3] * 2. - an[4] * 3. + ae[3]) * 6.22e-6
                  + cos(an[3] * 2. - an[4] * 3. + ae[2]) * 1.07e-6
                  - cos(an[1] - an[4]) * 4.31e-5
                  + cos(an[2] - an[4]) * -3.894e-5
                  - cos(an[3] - an[4]) * 8.011e-5
                  + cos(an[3] * 2. - an[4] * 2.) * 5.906e-5
                  + cos(an[3] * 3. - an[4] * 3.) * 3.749e-5
                  + cos(an[3] * 4. - an[4] * 4.) * 2.482e-5
                  + cos(an[3] * 5. - an[4] * 5.) * 1.684e-5;
        elements[1] =
                  - sin(an[2] - an[3] * 4. + an[4] * 3.) * 7.82e-6
                  + sin(an[3] * 2. - an[4] * 3. + ae[4]) * 5.129e-5
                  - sin(an[3] * 2. - an[4] * 3. + ae[3]) * 1.5824e-4
                  + sin(an[3] * 2. - an[4] * 3. + ae[2]) * 3.451e-5
                  + sin(an[1] - an[4]) * 4.751e-5
                  + sin(an[2] - an[4]) * 3.896e-5
                  + sin(an[3] - an[4]) * 3.5973e-4
                  + sin(an[3] * 2. - an[4] * 2.) * 2.8278e-4
                  + sin(an[3] * 3. - an[4] * 3.) * 1.386e-4
                  + sin(an[3] * 4. - an[4] * 4.) * 7.803e-5
                  + sin(an[3] * 5. - an[4] * 5.) * 4.729e-5
                  + sin(an[3] * 6. - an[4] * 6.) * 3e-5
                  + sin(an[3] * 7. - an[4] * 7.) * 1.962e-5
                  + sin(an[3] * 8. - an[4] * 8.) * 1.311e-5
                  + t * .46669212 - .9155918;
        elements[2] = cos(ae[1]) * -3.5e-7
                  + cos(ae[2]) * 7.453e-5
                  - cos(ae[3]) * 7.5868e-4
                  + cos(ae[4]) * .00139734
                  + cos(an[1]) * 3.9e-5
                  + cos(-an[1] + an[4] * 2.) * 1.766e-5
                  + cos(an[2]) * 3.242e-5
                  + cos(an[3]) * 7.975e-5
                  + cos(an[4]) * 7.566e-5
                  + cos(-an[3] + an[4] * 2.) * 1.3404e-4
                  - cos(an[3] * -2. + an[4] * 3.) * 9.8726e-4
                  - cos(an[3] * -3. + an[4] * 4.) * 1.2609e-4
                  - cos(an[3] * -4. + an[4] * 5.) * 5.742e-5
                  - cos(an[3] * -5. + an[4] * 6.) * 3.241e-5
                  - cos(an[3] * -6. + an[4] * 7.) * 1.999e-5
                  - cos(an[3] * -7. + an[4] * 8.) * 1.294e-5;
        elements[3] = sin(ae[1]) * -3.5e-7
                  + sin(ae[2]) * 7.453e-5
                  - sin(ae[3]) * 7.5868e-4
                  + sin(ae[4]) * .00139734
                  + sin(an[1]) * 3.9e-5
                  + sin(-an[1] + an[4] * 2.) * 1.766e-5
                  + sin(an[2]) * 3.242e-5
                  + sin(an[3]) * 7.975e-5
                  + sin(an[4]) * 7.566e-5
                  + sin(-an[3] + an[4] * 2.) * 1.3404e-4
                  - sin(an[3] * -2. + an[4] * 3.) * 9.8726e-4
                  - sin(an[3] * -3. + an[4] * 4.) * 1.2609e-4
                  - sin(an[3] * -4. + an[4] * 5.) * 5.742e-5
                  - sin(an[3] * -5. + an[4] * 6.) * 3.241e-5
                  - sin(an[3] * -6. + an[4] * 7.) * 1.999e-5
                  - sin(an[3] * -7. + an[4] * 8.) * 1.294e-5;
        elements[4] = cos(ai[0]) * -4.4e-7
                  - cos(ai[1]) * 3.1e-7
                  + cos(ai[2]) * 3.689e-5
                  - cos(ai[3]) * 5.9633e-4
                  + cos(ai[4]) * 4.5169e-4;
        elements[5] = sin(ai[0]) * -4.4e-7
                  - sin(ai[1]) * 3.1e-7
                  + sin(ai[2]) * 3.689e-5
                  - sin(ai[3]) * 5.9633e-4
                  + sin(ai[4]) * 4.5169e-4;
        break;
    }
}

static
const double gust86_rmu[5] = {
   1.291892353675174e-08,
   1.291910570526396e-08,
   1.291910102284198e-08,
   1.291942656265575e-08,
   1.291935967091320e-08};


const double GUST86toJ2000[9] = {
   9.753205572598290957e-01, 6.194437810676107434e-02, 2.119261772583629030e-01,
  -2.207428547845518695e-01, 2.529905336992995280e-01, 9.419492459363773150e-01,
   4.733143558215848563e-03,-9.654836528287313313e-01, 2.604206471702025216e-01
};

#if 0
const double GUST86toVsop87[9] = {
   9.753206632086812015e-01, 6.194425668001473004e-02, 2.119257251551559653e-01,
  -2.006444610981783542e-01,-1.519328516640849367e-01, 9.678110398294910731e-01,
   9.214881523275189928e-02,-9.864478281437795399e-01,-1.357544776485127136e-01
};
#endif


#if 0
void GetGust86OsculatingCoor(const double jd0,const double jd,
                             const int body,double *xyz) {
  double x[3];
  if (jd0 != gust86_jd0) {
    const double t0 = jd0 - 2444239.5;
    gust86_jd0 = jd0;
    CalcInterpolatedElements(t0,gust86_elem,
                             GUST86_DIM,
                             &CalcGust86Elem,DELTA_T,
                             &t_0,gust86_elem_0,
                             &t_1,gust86_elem_1,
                             &t_2,gust86_elem_2);
/*
    printf("GetGust86Coor(%d): %f %f  %f %f  %f %f\n",
           body,
           gust86_elem[body*6+0],gust86_elem[body*6+1],gust86_elem[body*6+2],
           gust86_elem[body*6+3],gust86_elem[body*6+4],gust86_elem[body*6+5]);
*/
  }
  EllipticToRectangularN(gust86_rmu[body],gust86_elem+(body*6),jd-jd0,x);
  xyz[0] = GUST86toVsop87[0]*x[0]+GUST86toVsop87[1]*x[1]+GUST86toVsop87[2]*x[2];
  xyz[1] = GUST86toVsop87[3]*x[0]+GUST86toVsop87[4]*x[1]+GUST86toVsop87[5]*x[2];
  xyz[2] = GUST86toVsop87[6]*x[0]+GUST86toVsop87[7]*x[1]+GUST86toVsop87[8]*x[2];
}
#endif


static void
EllipticToRectangular(const double a,const double n,
                      const double elem[6],const double dt,double xyz[])
{
    const double L = fmod(elem[1]+n*dt,2.0*M_PI);
    /* solve Keplers equation
        x = L - elem[2]*sin(x) + elem[3]*cos(x)
      not by trivially iterating
        x_0 = L
        x_{j+1} = L - elem[2]*sin(x_j) + elem[3]*cos(x_j)
      but instead by Newton approximation:
        0 = f(x) = x - L - elem[2]*sin(x) + elem[3]*cos(x)
        f'(x) = 1 - elem[2]*cos(x) - elem[3]*sin(x)
        x_0 = L or whatever, perhaps first step of trivial iteration
        x_{j+1} = x_j - f(x_j)/f'(x_j)
    */
    double Le = L - elem[2]*sin(L) + elem[3]*cos(L);
    for (;;)
    {
        const double cLe = cos(Le);
        const double sLe = sin(Le);
          /* for excenticity < 1 we have denominator > 0 */
        const double dLe = (L - Le + elem[2]*sLe - elem[3]*cLe)
                         / (1.0    - elem[2]*cLe - elem[3]*sLe);
        Le += dLe;
        if (fabs(dLe) <= 1e-14) break; /* L1: <1e-12 */
    }

    const double cLe = cos(Le);
    const double sLe = sin(Le);

    const double dlf = -elem[2]*sLe + elem[3]*cLe;
    const double phi = sqrt(1.0 - elem[2]*elem[2] - elem[3]*elem[3]);
    const double psi = 1.0 / (1.0 + phi);

    const double x1 = a * (cLe - elem[2] - psi*dlf*elem[3]);
    const double y1 = a * (sLe - elem[3] + psi*dlf*elem[2]);

    const double elem_4q = elem[4] * elem[4];
    const double elem_5q = elem[5] * elem[5];
    const double dwho = 2.0 * sqrt(1.0 - elem_4q - elem_5q);
    const double rtp = 1.0 - elem_5q - elem_5q;
    const double rtq = 1.0 - elem_4q - elem_4q;
    const double rdg = 2.0 * elem[5] * elem[4];

    xyz[0] = x1 * rtp + y1 * rdg;
    xyz[1] = x1 * rdg + y1 * rtq;
    xyz[2] = (-x1 * elem[5] + y1 * elem[4]) * dwho;

    const double rsam1 = -elem[2]*cLe - elem[3]*sLe;
    const double h = a*n / (1.0 + rsam1);
    const double vx1 = h * (-sLe - psi*rsam1*elem[3]);
    const double vy1 = h * ( cLe + psi*rsam1*elem[2]);

    xyz[3] = vx1 * rtp + vy1 * rdg;
    xyz[4] = vx1 * rdg + vy1 * rtq;
    xyz[5] = (-vx1 * elem[5] + vy1 * elem[4]) * dwho;
}


static void
EllipticToRectangularN(double mu,const double elem[6],double dt,
                       double xyz[])
{
    const double n = elem[0];
    const double a = pow(mu / (n * n), 1.0 / 3.0);
    EllipticToRectangular(a,n,elem,dt,xyz);
}



/** Compute the areocentric state of the satellite in the frame of the Earth mean
  * equator and equinox of J2000.
  */
StateVector
Gust86Orbit::state(double tdbSec) const
{
    const double GUST86_T0 = 2444239.5;

    double t = secondsToDays(tdbSec) + (J2000 - GUST86_T0);
    int satIndex = (int) m_satellite;

    double elements[6];
    CalcGust86Elem(t, m_satellite, elements);

    double x[6];
    EllipticToRectangularN(gust86_rmu[satIndex], elements, 0.0, x);

    const Matrix3d r = Matrix3d(GUST86toJ2000).transpose();

    // Transform the state vector from the Uranus equatorial coordinate system
    // to EMEJ2000 and convert units (position from AU to km, velocity from
    // AU/year to km/sec)
    Vector3d position = r * Vector3d(x[0], x[1], x[2]) * astro::AU;
    Vector3d velocity = r * Vector3d(x[3], x[4], x[5]) * astro::AU / daysToSeconds(1.0);

    return StateVector(position, velocity);
}


Gust86Orbit::Gust86Orbit(Satellite satellite) :
    m_satellite(satellite),
    m_boundingRadius(0.0),
    m_period(1.0)
{
}


#define TEST_GUST86 0

#if TEST_GUST86
static double TestStates[5][6] =
{
};
#endif


Gust86Orbit*
Gust86Orbit::Create(Satellite satellite)
{
    Gust86Orbit* orbit = new Gust86Orbit(satellite);

    int satIndex = (int) satellite;
    orbit->m_period = daysToSeconds(2.0 * PI / fqn[satIndex]);

    switch (satellite)
    {
    case Miranda:
        orbit->m_boundingRadius = 1.4e5;
        break;
    case Ariel:
        orbit->m_boundingRadius = 2.0e5;
        break;
    case Umbriel:
        orbit->m_boundingRadius = 2.7e5;
        break;
    case Titania:
        orbit->m_boundingRadius = 4.4e5;
        break;
    case Oberon:
        orbit->m_boundingRadius = 5.9e5;
        break;
    }

#if TEST_GUST86
    Vector3d testPos(TestStates[satIndex][0], TestStates[satIndex][1], TestStates[satIndex][2]);
    testPos *= astro::AU;
    std::cerr << "Mars sat test: " << (orbit->position(0.0) - testPos).norm() << " km\n";
#endif

    return orbit;
}