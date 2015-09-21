#pragma once

/*!
 * \file PixelFormat.h
 */


#include <glheadless/glheadless_api.h>

#include <map>


namespace glheadless {


/*!
 * \brief Describes the pixel format for a context.
 *
 * Offers methods to set a few common attributes. Other, platform-specific attributes can be set directly using
 * setAttribute(). Since the pixel format is technically required but does not matter for a headless context, the
 * defaults should usually be fine.
 */
class GLHEADLESS_API PixelFormat {
public:
    /*!
     * \brief Creates a PixelFormat with the default attributes.
     *
     * Default attributes are: 8 bit red, blue, green and alpha buffers, 24 bit depth buffer, 8 bit stencil buffer,
     * double buffering enabled and stereo disabled.
     */
    PixelFormat();

    /*!
     * \brief Sets all color buffer sizes in bits
     */
    void setColorBits(int redBits, int greenBits, int blueBits, int alphaBits);

    /*!
     * \return the red buffer size in bits
     */
    int redBits() const;

    /*!
     * \brief Sets the red buffer size in bits
     */
    void setRedBits(int redBits);

    /*!
     * \return the green buffer size in bits
     */
    int greenBits() const;

    /*!
     * \brief Sets the green buffer size in bits
     */
    void setGreenBits(int greenBits);

    /*!
     * \return the blue buffer size in bits
     */
    int blueBits() const;

    /*!
     * \brief Sets the blue buffer size in bits
     */
    void setBlueBits(int blueBits);

    /*!
     * \return the alpha buffer size in bits
     */
    int alphaBits() const;

    /*!
     * \brief Sets the alhpa buffer size in bits
     */
    void setAlphaBits(int alphaBits);

    /*!
     * \return the depth buffer size in bits
     */
    int depthBits() const;

    /*!
     * \brief Sets the depth buffer size in bits
     */
    void setDepthBits(int depthBits);

    /*!
     * \return the stencil buffer size in bits
     */
    int stencilBits() const;

    /*!
     * \brief Sets the stencil buffer size in bits
     */
    void setStencilBits(int stencilBits);

    /*!
     * \return true if double buffering is enanbled
     */
    bool doubleBuffer() const;

    /*!
     * \brief Enables/disables double buffering
     */
    void setDoubleBuffer(const bool doubleBuffer);

    /*!
     * \return true if stereoscopic rendering is enabled
     */
    bool stereo() const;

    /*!
     * \brief Enables/disables stereoscopic rendering
     */
    void setStereo(const bool stereo);

    /*!
     * \see setAttribute
     *
     * \return the raw attributes
     */
    const std::map<int, int>& attributes() const;

    /*!
     * \brief Sets a raw attribute.
     *
     * A raw attribute is a platform-dependent attribute that should be passed to the pixel format selection function
     * (e.g., wglChoosePixelFormatARB and CGLChoosePixelFormat). Any attributes specified here override attributes
     * derived from the settings above.
     * Boolean attributes are specified with their name and GL_FALSE/GL_TRUE as value, regardless of the actual
     * operating system API.
     */
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
