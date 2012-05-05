#pragma once

#include "../geometry/rect2d.hpp"

#include "../std/shared_ptr.hpp"
#include "../std/function.hpp"
#include "../std/vector.hpp"
#include "../std/queue.hpp"

namespace yg
{
  template <typename pair_t>
  struct first_less
  {
    bool operator()(pair_t const & first, pair_t const & second)
    {
      return first.first < second.first;
    }
  };

  namespace gl
  {
    class BaseTexture;
  }

  class SkinPage;
  class ResourceManager;
  struct ResourceStyle;
  struct PenInfo;
  struct CircleInfo;
  struct Color;
  struct GlyphKey;
  class GlyphCache;
  struct FontInfo;

  class Skin
  {
  public:

    typedef vector<shared_ptr<SkinPage> > TSkinPages;
    typedef function<void(uint8_t)> clearPageFn;
    typedef function<void(uint8_t)> overflowFn;

  private:

    TSkinPages m_pages;
    TSkinPages m_additionalPages;

    uint8_t m_dynamicPage;
    uint8_t m_textPage;

    uint8_t m_startStaticPage;
    uint8_t m_currentStaticPage;
    uint8_t m_staticPagesCount;

    shared_ptr<ResourceManager> m_resourceManager;

    bool m_fillAlpha;

    void clearHandles();

    Skin(shared_ptr<ResourceManager> const & resourceManager,
         TSkinPages const & pages);

    friend class SkinLoader;

    void addDynamicPages(int count);
    void addTextPages(int count);

    typedef pair<uint8_t, uint32_t> id_pair_t;
    id_pair_t unpackID(uint32_t id) const;
    uint32_t packID(uint8_t, uint32_t) const;

    typedef priority_queue<pair<size_t, clearPageFn>,
                           vector<pair<size_t, clearPageFn> >,
                           first_less<pair<size_t, clearPageFn> >
                           > clearPageFns;

    clearPageFns m_clearPageFns;
    void callClearPageFns(uint8_t pipelineID);

    typedef priority_queue<pair<size_t, overflowFn>,
                           vector<pair<size_t, overflowFn> >,
                           first_less<pair<size_t, overflowFn> >
                           > overflowFns;

    overflowFns m_overflowFns;
    void callOverflowFns(uint8_t pipelineID);

    void clearPageHandles(uint8_t pipelineID);
    void onDynamicOverflow(uint8_t pipelineID);
    void onTextOverflow(uint8_t pipelineID);

    void flushDynamicPage();
    void flushTextPage();

  public:

    /// clean and destroy
    ~Skin();
    /// obtain ResourceStyle from id
    ResourceStyle const * fromID(uint32_t id);
    /// get an identifier from the styleName.
    /// return 0xFFFF if this style is not found in Skin.
    uint32_t mapSymbol(char const * symbolName);
    /// find ruleDef on the texture.
    /// if found - return id.
    /// if not - pack and return id.
    uint32_t mapPenInfo(PenInfo const & penInfo);
    /// map an array of PenInfo on the same skin page
    /// returns the completion flag
    bool mapPenInfo(PenInfo const * penInfos, uint32_t * styleIDS, size_t count);
    /// find brushDef on the texture.
    /// if found - return id.
    /// if not - pack and return id.
    uint32_t mapColor(Color const & c);
    /// find glyph identified by GlyphKey on texture
    /// if found - return id
    /// if not - pack and return id
    uint32_t mapGlyph(GlyphKey const & gk, GlyphCache * glyphCache);
    /// find circleStyle on texture
    /// if found - return id
    /// if not - pack and return id
    uint32_t mapCircleInfo(CircleInfo const & circleInfo);

    void addClearPageFn(clearPageFn fn, int priority);
    void addOverflowFn(overflowFn, int priority);

    FontInfo const & getFont(int size) const;

    shared_ptr<SkinPage> const & getPage(int i) const;
    size_t getPagesCount() const;
    size_t getAdditionalPagesCount() const;

    uint32_t invalidHandle() const;
    uint32_t invalidPageHandle() const;

    uint8_t textPage() const;
    uint8_t dynamicPage() const;

    void setAdditionalPage(shared_ptr<SkinPage> const & pages);
    void clearAdditionalPage();

    void memoryWarning();
    void enterBackground();
    void enterForeground();
  };
}
