#include <glheadless/PixelFormat.h>


namespace glheadless {


PixelFormat::PixelFormat()
: m_redBits(8)
, m_greenBits(8)
, m_blueBits(8)
, m_alphaBits(8)
, m_depthBits(24)
, m_stencilBits(8)
, m_doubleBuffer(true)
, m_stereo(false) {
}


void PixelFormat::setColorBits(const int redBits, const int greenBits, const int blueBits, const int alphaBits) {
    m_redBits = redBits;
    m_greenBits = greenBits;
    m_blueBits = blueBits;
    m_alphaBits = alphaBits;
}


int PixelFormat::redBits() const {
    return m_redBits;
}


void PixelFormat::setRedBits(const int redBits) {
    m_redBits = redBits;
}

int PixelFormat::greenBits() const {
    return m_greenBits;
}


void PixelFormat::setGreenBits(const int greenBits) {
    m_greenBits = greenBits;
}


int PixelFormat::blueBits() const {
    return m_blueBits;
}


void PixelFormat::setBlueBits(const int blueBits) {
    m_blueBits = blueBits;
}


int PixelFormat::alphaBits() const {
    return m_alphaBits;
}


void PixelFormat::setAlphaBits(const int alphaBits) {
    m_alphaBits = alphaBits;
}


int PixelFormat::depthBits() const {
    return m_depthBits;
}


void PixelFormat::setDepthBits(const int depthBits) {
    m_depthBits = depthBits;
}


int PixelFormat::stencilBits() const {
    return m_stencilBits;
}


void PixelFormat::setStencilBits(const int stencilBits) {
    m_stencilBits = stencilBits;
}


bool PixelFormat::doubleBuffer() const {
    return m_doubleBuffer;
}


void PixelFormat::setDoubleBuffer(const bool doubleBuffer) {
    m_doubleBuffer = doubleBuffer;
}


bool PixelFormat::stereo() const {
    return m_stereo;
}


void PixelFormat::setStereo(const bool stereo) {
    m_stereo = stereo;
}


const std::map<int, int>& PixelFormat::attributes() const {
    return m_attributes;
}


void PixelFormat::setAttribute(int name, int value) {
    m_attributes[name] = value;
}


}  // namespace glheadless
