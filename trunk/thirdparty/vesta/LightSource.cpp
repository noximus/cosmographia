/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "LightSource.h"

using namespace vesta;


LightSource::LightSource() :
    m_luminosity(0.0f),
    m_spectrum(Spectrum::White()),
    m_range(1.0f),
    m_shadowCaster(false)
{
}
