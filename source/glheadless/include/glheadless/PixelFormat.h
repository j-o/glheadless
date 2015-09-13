#pragma once

#include <glheadless/glheadless_api.h>

#include <map>


namespace glheadless {


class GLHEADLESS_API PixelFormat {
public:
    PixelFormat();

    void setColorBits(int redBits, int greenBits, int blueBits, int alphaBits);

    int redBits() const;
    void setRedBits(int redBits);

    int greenBits() const;
    void setGreenBits(int greenBits);

    int blueBits() const;
    void setBlueBits(int blueBits);

    int alphaBits() const;
    void setAlphaBits(int alphaBits);

    int depthBits() const;
    void setDepthBits(int depthBits);

    int stencilBits() const;
    void setStencilBits(int stencilBits);

    bool doubleBuffer() const;
    void setDoubleBuffer(const bool doubleBuffer);

    bool stereo() const;
    void setStereo(const bool stereo);

    const std::map<int, int>& attributes() const;
    void setAttribute(int name, int value);


private:
    int m_redBits;
    int m_greenBits;
    int m_blueBits;
    int m_alphaBits;
    int m_depthBits;
    int m_stencilBits;
    bool m_doubleBuffer;
    bool m_stereo;
    std::map<int, int> m_attributes;
};


}  // namespace glheadless
